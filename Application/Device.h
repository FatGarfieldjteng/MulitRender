#pragma once

#include "CPUDescriptorAllocation.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <memory>

class Adapter;
class CPUDescriptorAllocator;
class CommandQueue;
class Texture;

class Device : public std::enable_shared_from_this<Device>
{
public:

    static std::shared_ptr<Device> create( std::shared_ptr<Adapter> adapter = nullptr );

    Microsoft::WRL::ComPtr<ID3D12Device2> device() const
    {
        return mDevice;
    }

    explicit Device(std::shared_ptr<Adapter> adapter);

    virtual ~Device();

public:
    // create resources
    
    // direct command queue
    ComPtr<ID3D12CommandQueue> createCommandQueue(D3D12_COMMAND_LIST_TYPE type);

    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, 
        uint32_t numDescriptors);

    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc);

    UINT getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

    ComPtr<ID3D12CommandAllocator> createCommandAllocator(D3D12_COMMAND_LIST_TYPE type);

    ComPtr<ID3D12GraphicsCommandList2> createCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator,
        D3D12_COMMAND_LIST_TYPE type);

    ComPtr<ID3D12Fence> Device::createFence();

public:
    // wrapper for native ID3D12Device2 APIs;
    void CreateRootSignature(const void* pBlobWithRootSignature,
        SIZE_T     blobLengthInBytes,
        REFIID     riid,
        void** ppvRootSignature
    );

    void createPipelineStateObject(const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
        REFIID riid,
        void** ppPipelineState);

    void createCommittedResource(
        const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS      HeapFlags,
        const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID                riidResource,
        void** ppvResource = nullptr);

    void copyDescriptors(UINT numDestDescriptorRanges,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
        const UINT* pDestDescriptorRangeSizes,
        UINT numSrcDescriptorRanges,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
        const UINT* pSrcDescriptorRangeSizes,
        D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapsType);

    void copyDescriptorsSimple(UINT numDescriptors,
        D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorRangeStart,
        D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
        D3D12_DESCRIPTOR_HEAP_TYPE  descriptorHeapsType);

    void createConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void createShaderResourceView(ID3D12Resource* pResource,
        const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void createUnorderedAccessView(
        ID3D12Resource* pResource,
        ID3D12Resource* pCounterResource,
        const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void createRenderTargetView(
        ID3D12Resource* pResource,
        const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void createDepthStencilView(
        ID3D12Resource* pResource,
        const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void createSampler(
        const D3D12_SAMPLER_DESC* pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

public:
    std::shared_ptr<Texture> createTexture(ComPtr<ID3D12Resource> resource);

    CPUDescriptorAllocation allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptors = 1U);

private:
    void createCommandQueues();
    void createCPUDescriptorAllocators();

private:
    Microsoft::WRL::ComPtr<ID3D12Device2> mDevice;

    // Default command queues.
    std::unique_ptr<CommandQueue> mDirectCommandQueue;
    std::unique_ptr<CommandQueue> mComputeCommandQueue;
    std::unique_ptr<CommandQueue> mCopyCommandQueue;

    // Descriptor allocators.
    std::unique_ptr<CPUDescriptorAllocator> mCPUDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};