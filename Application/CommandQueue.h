#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>
#include <queue>

class Device;

class CommdandQueue
{
public:

    static std::shared_ptr<CommdandQueue> create(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);

    explicit CommdandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);

    virtual ~CommdandQueue();

public:
    // get dx12 CommandQueue object
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue() const
    {
        return mCommandQueue;
    }
    
    // CommandList functions
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> acquireCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
    uint64_t executeCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

    // CommandAllocator functions
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> acquireCommandAllocator();

    // sync functions
    uint64_t signal();
    bool isFenceComplete(uint64_t fenceValue);
    void waitForFenceValue(uint64_t fenceValue);

    // flush
    void flush();

private:
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue< Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> >;

    D3D12_COMMAND_LIST_TYPE                     mCommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
    std::shared_ptr<Device>                     mDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>  mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence>         mFence;
    HANDLE                                      mFenceEvent = 0;
    uint64_t                                    mFenceValue = 0;

    CommandAllocatorQueue                       mCommandAllocatorQueue;
    CommandListQueue                            mCommandListQueue;
};