#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "ThreadSafeQueue.h"

#include <memory>
#include <queue>
#include <map>

class Device;
class CommandList;

class CommandQueue
{
public:

    // in constructor, command queue, fence, fence event are created 
    CommandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);

    virtual ~CommandQueue();

public:
    // get dx12 CommandQueue object
    ComPtr<ID3D12CommandQueue> commandQueue() const
    {
        return mCommandQueue;
    }
    
    // CommandList functions
    ComPtr<ID3D12GraphicsCommandList2> acquireDXCommandList();
    std::shared_ptr<CommandList> acquireCommandList();

    uint64_t executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList);
    uint64_t executeCommandList(std::shared_ptr<CommandList> commandList);
    uint64_t executeCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists);

    // sync functions
    uint64_t signal();
    bool isFenceComplete(uint64_t fenceValue);
    void waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max());

    // flush
    void flush();

protected:
    ComPtr<ID3D12CommandAllocator> createCommandAllocator();
    ComPtr<ID3D12GraphicsCommandList2> createCommandList(ComPtr<ID3D12CommandAllocator> allocator);

private:
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue< ComPtr<ID3D12GraphicsCommandList2> >;

    D3D12_COMMAND_LIST_TYPE                     mCommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
    std::shared_ptr<Device>                     mDevice;
    ComPtr<ID3D12CommandQueue>                  mCommandQueue;
    ComPtr<ID3D12Fence>                         mFence;
    HANDLE                                      mFenceEvent = 0;
    uint64_t                                    mFenceValue = 0;

    CommandAllocatorQueue                       mCommandAllocatorQueue;
    CommandListQueue                            mCommandListQueue;

    std::map<ComPtr<ID3D12GraphicsCommandList2>, ComPtr<ID3D12CommandAllocator>> mMapListAllocator;

    ThreadSafeQueue<std::shared_ptr<CommandList>> mAvailableCommandLists;
};