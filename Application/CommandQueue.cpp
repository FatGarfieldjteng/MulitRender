#include "framework.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Adapter.h"
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
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList2> commandList;

    // get a valid CommandAllocator, if there is free CommandAllocator available in mCommandAllocatorQueue.front(), use it
    // otherwise, create a new one
    if (!mCommandAllocatorQueue.empty() && isFenceComplete(mCommandAllocatorQueue.front().fenceValue))
    {
        commandAllocator = mCommandAllocatorQueue.front().commandAllocator;
        mCommandAllocatorQueue.pop();

        ThrowIfFailed(commandAllocator->Reset());
    }
    else
    {
        commandAllocator = createCommandAllocator();
    }

    // get a valie CommandList, if there is free CommandList available in mCommandListQueue.front(), use it
    // otherwise, create a new one
    if (!mCommandListQueue.empty())
    {
        commandList = mCommandListQueue.front();
        mCommandListQueue.pop();

        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
    }
    else
    {
        commandList = createCommandList(commandAllocator);
        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
    }

    //mMapListAllocator[commandList] = commandAllocator;
    
    
    // Associate the command allocator with the command list so that it can be
    // retrieved when the command list is executed.
    ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));
    
    return commandList;
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

uint64_t CommandQueue::executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->Close();

    ID3D12CommandAllocator* commandAllocator;
    UINT dataSize = sizeof(commandAllocator);
    ThrowIfFailed(commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* const ppCommandLists[] = {
        commandList.Get()
    };

    mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    uint64_t fenceValue = signal();

    mCommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
    mCommandListQueue.push(commandList);

    commandAllocator->Release();

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