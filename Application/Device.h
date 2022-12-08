#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

class Adapter;

class Device
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

    void createDepthStencilView(
        ID3D12Resource * pResource,
        const D3D12_DEPTH_STENCIL_VIEW_DESC * pDesc,
        D3D12_CPU_DESCRIPTOR_HANDLE   DestDescriptor);

private:
    Microsoft::WRL::ComPtr<ID3D12Device2> mDevice;
};