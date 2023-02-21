#include "framework.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Adapter.h"
#include "ResourceStateTracker.h"
#include "Device.h"
#include "helper.h"

CommandQueue::CommandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
    :mDevice(device), mCommandListType(type)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    mCommandQueue = mDevice->createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    mFence = mDevice->createFence();

    mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(mFenceEvent && "Failed to create fence event.");
}

CommandQueue::~CommandQueue()
{
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::acquireDXCommandList()
{
    return acquireCommandList()->commandList();

}

std::shared_ptr<CommandList> CommandQueue::acquireCommandList()
{
    std::shared_ptr<CommandList> commandList;

    // If there is a command list on the queue.
    if (!mAvailableCommandLists.isEmpty())
    {
        mAvailableCommandLists.tryPop(commandList);
    }
    else
    {
        // Otherwise create a new command list.
        commandList = std::make_shared<CommandList>(mDevice, mCommandListType);
    }

    return commandList;
}

void CommandQueue::executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandList* const ppCommandLists[] = {
        commandList.Get()
    };

    mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    uint64_t fenceValue = signal();

}

uint64_t CommandQueue::executeCommandListAndSignal(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandList* const ppCommandLists[] = {
        commandList.Get()
    };

    mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    uint64_t fenceValue = signal();

    return fenceValue;
}

uint64_t CommandQueue::executeCommandListsAndSignal(std::vector<ID3D12GraphicsCommandList2*>& commandLists)
{
    
    for (ID3D12GraphicsCommandList2* cl : commandLists)
    {
        cl->Close();
    }

    mCommandQueue->ExecuteCommandLists(commandLists.size(), (ID3D12CommandList* const*)(commandLists.data()));
    uint64_t fenceValue = signal();

    return fenceValue;
}


// sync functions
uint64_t CommandQueue::signal()
{
    uint64_t fenceValueForSignal = ++mFenceValue;
    ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

bool CommandQueue::isFenceComplete(uint64_t fenceValue)
{
    return (mFence->GetCompletedValue() >= fenceValue);
}

void CommandQueue::waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration)
{
    if (mFence->GetCompletedValue() < fenceValue)
    {
        ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
        ::WaitForSingleObject(mFenceEvent, static_cast<DWORD>(duration.count()));
    }
}

// flush
void CommandQueue::flush()
{
    uint64_t fenceValueForSignal = signal();
    waitForFenceValue(fenceValueForSignal);
}

ComPtr<ID3D12CommandAllocator> CommandQueue::createCommandAllocator()
{
    return mDevice->createCommandAllocator(mCommandListType);
}

ComPtr<ID3D12GraphicsCommandList2> CommandQueue::createCommandList(ComPtr<ID3D12CommandAllocator> allocator)
{
    return mDevice->createCommandList(allocator, mCommandListType);
}