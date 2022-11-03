#pragma once

#include <wrl/client.h>  // For Microsoft::WRL::ComPtr
#include <d3d12.h>
#include "SwapChain.h"
#include "Device.h"

#include <memory>
#include <string>

class GraphicsSystem
{
public:

	static const UINT BufferCount = 3;

	GraphicsSystem();
	~GraphicsSystem();

public:

	void initGraphicsSystem(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

	void createDevice();

	void createDirectCommandQueue();

	void createSwapChain(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

	void createEventHandle();
	
	uint64_t signal();

	void waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max());

	void flush(uint64_t& fenceValue);

	void Update();

	void Render();


public:
	std::shared_ptr<Adapter> mAdapter;
	std::shared_ptr<Device> mDevice;
	std::shared_ptr<SwapChain> mSwapChain;

	ComPtr<ID3D12CommandAllocator> mCommandAllocators[BufferCount];
	ComPtr<ID3D12GraphicsCommandList> mCommandList;

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12Fence> mFence;
	uint64_t mFenceValue = 0;

	// TODO: Move the following to SwapChain class
	ComPtr<ID3D12DescriptorHeap> mRTVDescriptorHeap;
	UINT mRTVDescriptorSize;
	UINT mCurrentBackBufferIndex;

	uint64_t mFrameFenceValues[BufferCount] = {};

	HANDLE mFenceEvent;
};