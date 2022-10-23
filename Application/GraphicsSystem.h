#pragma once

#include <wrl/client.h>  // For Microsoft::WRL::ComPtr
#include <d3d12.h>
#include "SwapChain.h"
#include "Device.h"

#include <memory>

class GraphicsSystem
{
public:

	GraphicsSystem();
	~GraphicsSystem();

public:
	void createDevice();

	void createDirectCommandQueue();

	void createSwapChain(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

public:
	std::shared_ptr<Adapter> mAdapter;
	std::shared_ptr<Device> mDevice;
	std::shared_ptr<SwapChain> mSwapChain;

};