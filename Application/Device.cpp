#include "framework.h"
#include "Device.h"
#include "Adapter.h"
#include "Texture.h"
#include "CPUDescriptorAllocator.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "helper.h"

std::shared_ptr<Device> Device::create( std::shared_ptr<Adapter> adapter )
{
    return std::make_shared<Device>(adapter);
}

Device::Device( std::shared_ptr<Adapter> adapter )
{
    auto dxgiAdapter = adapter->adapter();

    ThrowIfFailed( D3D12CreateDevice( dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS( &mDevice) ) );

    // Enable debug messages (only works if the debug layer has already been enabled).
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if ( SUCCEEDED(mDevice.As( &pInfoQueue ) ) )
    {

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,  // I'm really not sure how to avoid this
                                                                           // message.

            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                     // debugging.

            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                       // debugging.

           // Workarounds for debug layer issues on hybrid-graphics systems
           D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE,
           D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        // NewFilter.DenyList.NumCategories = _countof(Categories);
        // NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof( Severities );
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs        = _countof( DenyIds );
        NewFilter.DenyList.pIDList       = DenyIds;

        ThrowIfFailed( pInfoQueue->PushStorageFilter( &NewFilter ) );
    }

    //// create commmand queues
    //createCommandQueues();

    //// create GPU descriptor allocators
    //createCPUDescriptorAllocators();
}

Device::~Device() 
{
}

ComPtr<ID3D12CommandQueue> Device::createCommandQueue(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> commandQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

    return commandQueue;
}

ComPtr<ID3D12DescriptorHeap> Device::createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptors,
    D3D12_DESCRIPTOR_HEAP_FLAGS Flags)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;
    desc.Flags = Flags;

    ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

ComPtr<ID3D12DescriptorHeap> Device::createDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(mDevice->CreateDescriptorHeap(pDescriptorHeapDesc, 
        IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

UINT Device::getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    return mDevice->GetDescriptorHandleIncrementSize(type);
}

ComPtr<ID3D12CommandAllocator> Device::createCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList2> Device::createCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, 
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    ThrowIfFailed(mDevice->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    //ThrowIfFailed(commandList->Close());

    return commandList;
}

std::unique_ptr<CommandList> Device::createUniqueCommandList(D3D12_COMMAND_LIST_TYPE type)
{
    return std::make_unique<CommandList>(shared_from_this(), type);
}

ComPtr<ID3D12Fence> Device::createFence()
{
    ComPtr<ID3D12Fence> fence;

    ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

void Device::CreateRootSignature(const void* pBlobWithRootSignature,
    SIZE_T     blobLengthInBytes,
    REFIID     riid,
    void** ppvRootSignature
)
{
    ThrowIfFailed(mDevice->CreateRootSignature(0, pBlobWithRootSignature,
        blobLengthInBytes, riid, ppvRootSignature));
}

void Device::createCommittedResource(
    const D3D12_HEAP_PROPERTIES* pHeapProperties,
    D3D12_HEAP_FLAGS      HeapFlags,
    const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES InitialResourceState,
    const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    REFIID                riidResource,
    void** ppvResource)
{
    ThrowIfFailed(mDevice->CreateCommittedResource(
        pHeapProperties,
        HeapFlags,
        pDesc,
        InitialResourceState,
        pOptimizedClearValue,
        riidResource,
        ppvResource));
}

void Device::createPipelineStateObject(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
    REFIID riid,
    void** ppPipelineState)
{
    mDevice->CreatePipelineState(pDesc, riid, ppPipelineState);
}

void Device::copyDescriptors(UINT numDestDescriptorRanges,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
    const UINT* pDestDescriptorRangeSizes,
    UINT numSrcDescriptorRanges,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
    const UINT* pSrcDescriptorRangeSizes,
    D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapsType)
{
    mDevice->CopyDescriptors(numDestDescriptorRanges,
        pDestDescriptorRangeStarts,
        pDestDescriptorRangeSizes,
        numSrcDescriptorRanges,
        pSrcDescriptorRangeStarts,
        pSrcDescriptorRangeSizes,
        descriptorHeapsType);
}

void Device::copyDescriptorsSimple(UINT numDescriptors,
    D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorRangeStart,
    D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
    D3D12_DESCRIPTOR_HEAP_TYPE  descriptorHeapsType)
{
    mDevice->CopyDescriptorsSimple(numDescriptors,
        destDescriptorRangeStart,
        SrcDescriptorRangeStart,
        descriptorHeapsType);
}

void Device::createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateConstantBufferView(pDesc,
        DestDescriptor);
}

void Device::createShaderResourceView(ID3D12Resource* pResource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateShaderResourceView(pResource,
        pDesc,
        DestDescriptor);
}

void Device::createUnorderedAccessView(
    ID3D12Resource* pResource,
    ID3D12Resource* pCounterResource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateUnorderedAccessView(
        pResource,
        pCounterResource,
        pDesc,
        DestDescriptor);
}

void Device::createRenderTargetView(
    ID3D12Resource* pResource,
    const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateRenderTargetView(
        pResource,
        pDesc,
        DestDescriptor);
}

void Device::createDepthStencilView(
    ID3D12Resource* pResource,
    const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateDepthStencilView(
        pResource,
        pDesc,
        DestDescriptor);
}

void Device::createSampler(
    const D3D12_SAMPLER_DESC* pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    mDevice->CreateSampler(
        pDesc,
        DestDescriptor);
}

void Device::createCommandQueues()
{
    mDirectCommandQueue = std::make_unique<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    mComputeCommandQueue = std::make_unique<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
    mCopyCommandQueue = std::make_unique<CommandQueue>(shared_from_this(), D3D12_COMMAND_LIST_TYPE_COPY);
}

void Device::createCPUDescriptorAllocators()
{
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        mCPUDescriptorAllocators[i] =
            std::make_unique<CPUDescriptorAllocator>(shared_from_this(), static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
    }
}

std::shared_ptr<Texture> Device::createTexture(ComPtr<ID3D12Resource> resource)
{
    return std::make_shared<Texture>(shared_from_this(), resource);
}

//
CPUDescriptorAllocation Device::allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
    return mCPUDescriptorAllocators[type]->allocate(numDescriptors);
}