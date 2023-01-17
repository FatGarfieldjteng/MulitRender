#include "framework.h"
#include "CommandRecorder.h"
#include "Device.h"

CommandRecorder::CommandRecorder(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
 
{
    mCommandAllocator = device->createCommandAllocator(type);

    mCommandList = device->createCommandList(mCommandAllocator, type);
}

CommandRecorder::~CommandRecorder()
{}