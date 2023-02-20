#include "framework.h"
#include "CommandList.h"
#include "Device.h"
#include "UploadBuffer.h"
#include "ViewManager.h"
#include "GraphicsResource.h"
#include "ResourceStateTracker.h"
#include "Texture.h"
#include "helper.h"
#include <DirectXTex.h>
#include <filesystem>

namespace fs = std::filesystem;

std::map<std::wstring, ID3D12Resource*> CommandList::msTextureCache;
std::mutex                              CommandList::msTextureCacheMutex;

CommandList::CommandList(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
    : mDevice(device)
    , mCommandListType(type)
{
    mCommandAllocator = mDevice->createCommandAllocator(mCommandListType);

    mCommandList = mDevice->createCommandList(mCommandAllocator, mCommandListType);

    mUploadBuffer = std::make_unique<UploadBuffer>(mDevice);

    mResourceStateTracker = std::make_unique<ResourceStateTracker>();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        mViewManagers[i] = std::make_unique<ViewManager>(mDevice, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
        mDescriptorHeaps[i] = nullptr;
    }
}

CommandList::~CommandList()
{}

void CommandList::descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
{
    if (mDescriptorHeaps[heapType] != heap)
    {
        mDescriptorHeaps[heapType] = heap;
        bindDescriptorHeaps();
    }
}

std::shared_ptr<Texture> CommandList::loadTextureFromFile(const std::wstring& fileName, bool sRGB)
{
    std::shared_ptr<Texture> texture;
    fs::path                 filePath(fileName);
    if (!fs::exists(filePath))
    {
        throw std::exception("File not found.");
    }

    std::lock_guard<std::mutex> lock(msTextureCacheMutex);
    auto iter = msTextureCache.find(fileName);
    if (iter != msTextureCache.end())
    {
        // not in cache, create texture
    }
    else
    {
        // load image from file
        DirectX::TexMetadata  metadata;
        DirectX::ScratchImage scratchImage;

        if (filePath.extension() == ".dds")
        {
            ThrowIfFailed(DirectX::LoadFromDDSFile(fileName.c_str(), DirectX::DDS_FLAGS_FORCE_RGB, &metadata, scratchImage));
        }
        else if (filePath.extension() == ".hdr")
        {
            ThrowIfFailed(DirectX::LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
        }
        else if (filePath.extension() == ".tga")
        {
            ThrowIfFailed(DirectX::LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
        }
        else
        {
            ThrowIfFailed(DirectX::LoadFromWICFile(fileName.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, &metadata, scratchImage));
        }

        // set sRGB format for linearization if necessary
        if (sRGB)
        {
            metadata.format = DirectX::MakeSRGB(metadata.format);
        }

        // create D3D12_RESOURCE_DESC for generating texture
        D3D12_RESOURCE_DESC textureDesc = {};
        switch (metadata.dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE1D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format, static_cast<UINT64>(metadata.width),
                static_cast<UINT16>(metadata.arraySize));
            break;
        case DirectX::TEX_DIMENSION_TEXTURE2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, static_cast<UINT64>(metadata.width),
                static_cast<UINT>(metadata.height),
                static_cast<UINT16>(metadata.arraySize));
            break;
        case DirectX::TEX_DIMENSION_TEXTURE3D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format, static_cast<UINT64>(metadata.width),
                static_cast<UINT>(metadata.height),
                static_cast<UINT16>(metadata.depth));
            break;
        default:
            throw std::exception("Invalid texture dimension.");
            break;
        }
                
        ComPtr<ID3D12Resource> textureResource;

        // create Resource to store texture data, while created, the resource data is empty
        mDevice->createCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
            D3D12_HEAP_FLAG_NONE, 
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON, 
            nullptr, 
            IID_PPV_ARGS(&textureResource));

        // create texture and views
        texture = mDevice->createTexture(textureResource);
        texture->mName = fileName;

        // add to global state tracker map
        ResourceStateTracker::addGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

        // set image data
        std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
        const DirectX::Image* pImages = scratchImage.GetImages();
        for (int i = 0; i < scratchImage.GetImageCount(); ++i)
        {
            auto& subresource = subresources[i];
            subresource.RowPitch = pImages[i].rowPitch;
            subresource.SlicePitch = pImages[i].slicePitch;
            subresource.pData = pImages[i].pixels;
        }

        copyTextureSubresource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

        if (subresources.size() < textureResource->GetDesc().MipLevels)
        {
            generateMips(texture);
        }

        // add to cache
        msTextureCache[fileName] = textureResource.Get();
    } // else

    return texture;
}

void CommandList::copyTextureSubresource(const std::shared_ptr<Texture>& texture, 
    uint32_t firstSubresource,
    uint32_t numSubresources, 
    D3D12_SUBRESOURCE_DATA* subresourceData)
{

    auto destinationResource = texture->mResource;

    if (destinationResource)
    {
        // Resource must be in the copy-destination state.
        transitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
        flushResourceBarriers();

        UINT64 requiredSize =
            GetRequiredIntermediateSize(destinationResource.Get(), firstSubresource, numSubresources);

        // Create a temporary (intermediate) resource for uploading the subresources
        ComPtr<ID3D12Resource> intermediateResource;
        mDevice->createCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(requiredSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&intermediateResource));

        UpdateSubresources(mCommandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0,
            firstSubresource, numSubresources, subresourceData);

        trackResource(intermediateResource);
        trackResource(destinationResource);
    }
}

void CommandList::generateMips(const std::shared_ptr<Texture>& texture)
{
    if (!texture)
        return;

    //auto d3d12Device = m_Device.GetD3D12Device();

    //if (mCommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
    //{
    //    if (!mComputeCommandList)
    //    {
    //        mComputeCommandList = mDevice.getCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE).GetCommandList();
    //    }
    //    mComputeCommandList->generateMips(texture);
    //    return;
    //}

    //auto d3d12Resource = texture->GetD3D12Resource();

    //// If the texture doesn't have a valid resource? Do nothing...
    //if (!d3d12Resource)
    //    return;
    //auto resourceDesc = d3d12Resource->GetDesc();

    //// If the texture only has a single mip level (level 0)
    //// do nothing.
    //if (resourceDesc.MipLevels == 1)
    //    return;
    //// Currently, only non-multi-sampled 2D textures are supported.
    //if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || resourceDesc.DepthOrArraySize != 1 ||
    //    resourceDesc.SampleDesc.Count > 1)
    //{
    //    throw std::exception("GenerateMips is only supported for non-multi-sampled 2D Textures.");
    //}

    //ComPtr<ID3D12Resource> uavResource = d3d12Resource;
    //// Create an alias of the original resource.
    //// This is done to perform a GPU copy of resources with different formats.
    //// BGR -> RGB texture copies will fail GPU validation unless performed
    //// through an alias of the BRG resource in a placed heap.
    //ComPtr<ID3D12Resource> aliasResource;

    //// If the passed-in resource does not allow for UAV access
    //// then create a staging resource that is used to generate
    //// the mipmap chain.
    //if (!texture->CheckUAVSupport() || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
    //{
    //    // Describe an alias resource that is used to copy the original texture.
    //    auto aliasDesc = resourceDesc;
    //    // Placed resources can't be render targets or depth-stencil views.
    //    aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    //    aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    //    // Describe a UAV compatible resource that is used to perform
    //    // mipmapping of the original texture.
    //    auto uavDesc = aliasDesc;  // The flags for the UAV description must match that of the alias description.
    //    uavDesc.Format = Texture::GetUAVCompatableFormat(resourceDesc.Format);

    //    D3D12_RESOURCE_DESC resourceDescs[] = { aliasDesc, uavDesc };

    //    // Create a heap that is large enough to store a copy of the original resource.
    //    auto allocationInfo = d3d12Device->GetResourceAllocationInfo(0, _countof(resourceDescs), resourceDescs);

    //    D3D12_HEAP_DESC heapDesc = {};
    //    heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
    //    heapDesc.Alignment = allocationInfo.Alignment;
    //    heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
    //    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    //    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    //    heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    //    ComPtr<ID3D12Heap> heap;
    //    ThrowIfFailed(d3d12Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

    //    // Make sure the heap does not go out of scope until the command list
    //    // is finished executing on the command queue.
    //    TrackResource(heap);

    //    // Create a placed resource that matches the description of the
    //    // original resource. This resource is used to copy the original
    //    // texture to the UAV compatible resource.
    //    ThrowIfFailed(d3d12Device->CreatePlacedResource(heap.Get(), 0, &aliasDesc, D3D12_RESOURCE_STATE_COMMON,
    //        nullptr, IID_PPV_ARGS(&aliasResource)));

    //    ResourceStateTracker::AddGlobalResourceState(aliasResource.Get(), D3D12_RESOURCE_STATE_COMMON);
    //    // Ensure the scope of the alias resource.
    //    TrackResource(aliasResource);

    //    // Create a UAV compatible resource in the same heap as the alias
    //    // resource.
    //    ThrowIfFailed(d3d12Device->CreatePlacedResource(heap.Get(), 0, &uavDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
    //        IID_PPV_ARGS(&uavResource)));

    //    ResourceStateTracker::AddGlobalResourceState(uavResource.Get(), D3D12_RESOURCE_STATE_COMMON);

    //    // Ensure the scope of the UAV compatible resource.
    //    TrackResource(uavResource);

    //    // Add an aliasing barrier for the alias resource.
    //    AliasingBarrier(nullptr, aliasResource);

    //    // Copy the original resource to the alias resource.
    //    // This ensures GPU validation.
    //    CopyResource(aliasResource, d3d12Resource);

    //    // Add an aliasing barrier for the UAV compatible resource.
    //    AliasingBarrier(aliasResource, uavResource);
    //}

    //// Generate mips with the UAV compatible resource.
    //auto uavTexture = m_Device.CreateTexture(uavResource);
    //GenerateMips_UAV(uavTexture, Texture::IsSRGBFormat(resourceDesc.Format));

    //if (aliasResource)
    //{
    //    AliasingBarrier(uavResource, aliasResource);
    //    // Copy the alias resource back to the original resource.
    //    CopyResource(d3d12Resource, aliasResource);
    //}
}

void CommandList::bindDescriptorHeaps()
{
    UINT numDescriptorHeaps = 0;
    ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

    for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        ID3D12DescriptorHeap* descriptorHeap = mDescriptorHeaps[i];
        if (descriptorHeap)
        {
            descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
        }
    }

    mCommandList->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
}

void CommandList::transitionBarrier(ComPtr<ID3D12Resource> resource, 
    D3D12_RESOURCE_STATES stateAfter,
    UINT subresource, 
    bool flushBarriers)
{
    if (resource)
    {
        // The "before" state is not important. It will be resolved by the resource state tracker.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter,
            subresource);
        mResourceStateTracker->resourceBarrier(barrier);
    }

    if (flushBarriers)
    {
        flushResourceBarriers();
    }
}

void CommandList::transitionResource(ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState,
    D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    mCommandList->ResourceBarrier(1, &barrier);
}

void CommandList::clearRTV(FLOAT* clearValue, D3D12_CPU_DESCRIPTOR_HANDLE rtv)
{
    mCommandList->ClearRenderTargetView(rtv, clearValue, 0, nullptr);
}

void CommandList::clearDepth(D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void CommandList::setPipelineState(ID3D12PipelineState* pPipelineState)
{
    mCommandList->SetPipelineState(pPipelineState);
}

void CommandList::setGraphicsRootSignature(ID3D12RootSignature* pRootSignature)
{
    mCommandList->SetGraphicsRootSignature(pRootSignature);
}

void CommandList::setDescriptorHeaps(UINT NumDescriptorHeaps,
    ID3D12DescriptorHeap* const* ppDescriptorHeaps)
{
    mCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
}

void CommandList::setGraphicsRootDescriptorTable(UINT RootParameterIndex,
    D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    mCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void CommandList::RSSetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports)
{
    mCommandList->RSSetViewports(NumViewports, pViewports);
}

void CommandList::RSSetScissorRects(const D3D12_RECT* pRects, UINT NumRects)
{
    mCommandList->RSSetScissorRects(NumRects, pRects);
}


void CommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
    mCommandList->IASetPrimitiveTopology(PrimitiveTopology);
}

void CommandList::OMSetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor,
    UINT NumRenderTargetDescriptors,
    BOOL RTsSingleHandleToDescriptorRange)
{
    mCommandList->OMSetRenderTargets(NumRenderTargetDescriptors, 
        pRenderTargetDescriptors, 
        RTsSingleHandleToDescriptorRange, 
        pDepthStencilDescriptor);
}

void CommandList::setGraphicsRoot32BitConstants(UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    mCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex,
        Num32BitValuesToSet, 
        pSrcData, 
        DestOffsetIn32BitValues);
}

void CommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
{
    mCommandList->IASetIndexBuffer(pView);
}

void CommandList::IASetVertexBuffers(UINT StartSlot,
    UINT NumViews,
    const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
    mCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void CommandList::drawIndexedInstanced(UINT IndexCountPerInstance,
    UINT InstanceCount,
    UINT StartIndexLocation,
    INT BaseVertexLocation,
    UINT StartInstanceLocation)
{
    mCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void CommandList::transitionBarrier(const std::shared_ptr<GraphicsResource>& resource,
    D3D12_RESOURCE_STATES stateAfter, 
    UINT subResource, 
    bool flushBarriers)
{
    auto dxResource = resource->mResource;
    transitionBarrier(dxResource,
        stateAfter,
        subResource,
        flushBarriers);
   
}

void CommandList::UAVBarrier(const GraphicsResource& resource, bool flushBarriers)
{
    auto dxResource = resource.mResource;
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(dxResource.Get());

    mResourceStateTracker->resourceBarrier(barrier);

    if (flushBarriers)
    {
        flushResourceBarriers();
    }
}

void CommandList::AliasingBarrier(const GraphicsResource& beforeResource, const GraphicsResource& afterResource, bool flushBarriers)
{
    auto d3d12BeforeResource = beforeResource.mResource;
    auto d3d12AfterResource = afterResource.mResource;
    auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(d3d12BeforeResource.Get(), d3d12AfterResource.Get());

    mResourceStateTracker->resourceBarrier(barrier);

    if (flushBarriers)
    {
        flushResourceBarriers();
    }
}

void CommandList::copyResource(ComPtr<ID3D12Resource> dstRes,
    ComPtr<ID3D12Resource> srcRes)
{
    transitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
    transitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

    flushResourceBarriers();

    mCommandList->CopyResource(dstRes.Get(), srcRes.Get());

    trackResource(dstRes);
    trackResource(srcRes);
}

void CommandList::flushResourceBarriers()
{
    mResourceStateTracker->flushResourceBarriers(*this);
}

void CommandList::trackResource(ComPtr<ID3D12Object> object)
{
    mTrackedObjects.push_back(object);
}

void CommandList::trackResource(const std::shared_ptr<GraphicsResource>& res)
{
    trackResource(res->mResource);
}

void CommandList::setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, 
    size_t sizeInBytes, 
    const void* bufferData)
{
    // Constant buffers must be 256-byte aligned.
    UploadBuffer::Allocation allocation = mUploadBuffer->allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    memcpy(allocation.CPUAddress, bufferData, sizeInBytes);

    mCommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, allocation.GPUAddress);
}

//void CommandList::setShaderResourceView(uint32_t rootParameterIndex,
//    uint32_t descriptorOffset,
//    const GraphicsResource& resource,
//    D3D12_RESOURCE_STATES stateAfter,
//    UINT firstSubresource,
//    UINT numSubresources,
//    const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
//{
//    if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
//    {
//        for (uint32_t i = 0; i < numSubresources; ++i)
//        {
//            transitionBarrier(resource, stateAfter, firstSubresource + i);
//        }
//    }
//    else
//    {
//        transitionBarrier(resource, stateAfter);
//    }
//
//    //mViewManagers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stageCPUDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));
//
//    trackResource(resource);
//}

void CommandList::draw(uint32_t vertexCount, 
    uint32_t instanceCount, 
    uint32_t startVertex, 
    uint32_t startInstance)
{
    flushResourceBarriers();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        mViewManagers[i]->commitStagedDescriptorsForDraw(*this);
    }

    mCommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void CommandList::drawIndexed(uint32_t indexCount, 
    uint32_t instanceCount, 
    uint32_t startIndex, 
    int32_t baseVertex,
    uint32_t startInstance)
{
    flushResourceBarriers();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        mViewManagers[i]->commitStagedDescriptorsForDraw(*this);
    }

    mCommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
}