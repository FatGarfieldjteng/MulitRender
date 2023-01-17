#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <memory>

class Device;

class CommandRecorder
{
public:
	CommandRecorder(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);
	~CommandRecorder();

private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
};