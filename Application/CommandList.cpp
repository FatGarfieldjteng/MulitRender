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

    mDevice->createCommandList(mCommandAllocator, mCommandListType);

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

        mDevice->createCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
            D3D12_HEAP_FLAG_NONE, 
            &textureDesc,
            D3D12_RESOURCE_STATE_COMMON, 
            nullptr, 
            IID_PPV_ARGS(&textureResource));

        texture = mDevice->createTexture(textureResource);
        texture->mName = fileName;
    }
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

void CommandList::transitionBarrier(const GraphicsResource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
{
    auto dxResource = resource.mResource;
    if (dxResource)
    {
        // The "before" state is not important. It will be resolved by the resource state tracker.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dxResource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource);
        mResourceStateTracker->resourceBarrier(barrier);
    }

    if (flushBarriers)
    {
        flushResourceBarriers();
    }
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

void CommandList::copyResource(GraphicsResource& dstRes, const GraphicsResource& srcRes)
{
    transitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
    transitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

    flushResourceBarriers();

    mCommandList->CopyResource(dstRes.mResource.Get(), srcRes.mResource.Get());

    trackResource(dstRes);
    trackResource(srcRes);
}

void CommandList::flushResourceBarriers()
{
    mResourceStateTracker->flushResourceBarriers(*this);
}

void CommandList::trackResource(const GraphicsResource& res)
{
    trackObject(res.mResource);
}

void CommandList::trackObject(ComPtr<ID3D12Object> object)
{
    mTrackedObjects.push_back(object);
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

void CommandList::setShaderResourceView(uint32_t rootParameterIndex,
    uint32_t descriptorOffset,
    const GraphicsResource& resource,
    D3D12_RESOURCE_STATES stateAfter,
    UINT firstSubresource,
    UINT numSubresources,
    const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
{
    if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
    {
        for (uint32_t i = 0; i < numSubresources; ++i)
        {
            transitionBarrier(resource, stateAfter, firstSubresource + i);
        }
    }
    else
    {
        transitionBarrier(resource, stateAfter);
    }

    //mViewManagers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->stageCPUDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));

    trackResource(resource);
}

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