#include "framework.h"
#include "CommandQueue.h"
#include "Adapter.h"
#include "Device.h"
#include "helper.h"

std::shared_ptr<CommdandQueue> CommdandQueue::create(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
{
    return std::make_shared<CommdandQueue>(device, type);
}

CommdandQueue::CommdandQueue(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
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

CommdandQueue::~CommdandQueue()
{
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommdandQueue::acquireCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{

}

uint64_t CommdandQueue::executeCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList)
{

}

// CommandAllocator functions
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommdandQueue::acquireCommandAllocator()
{

}

// sync functions
uint64_t CommdandQueue::signal()
{

}

bool CommdandQueue::isFenceComplete(uint64_t fenceValue)
{

}

void CommdandQueue::waitForFenceValue(uint64_t fenceValue)
{

}

// flush
void CommdandQueue::flush()
{

}