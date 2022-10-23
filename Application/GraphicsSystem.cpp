#include "framework.h"
#include "GraphicsSystem.h"
#include "Adapter.h"
#include "helper.h"

GraphicsSystem::GraphicsSystem()
{

}

GraphicsSystem::~GraphicsSystem()
{

}

void GraphicsSystem::createDevice()
{
	mAdapter = Adapter::Create();

	mDevice = Device::create(mAdapter);
	 
}

void GraphicsSystem::createDirectCommandQueue()
{
	mDevice->createDirectCommandQueue();
}

void GraphicsSystem::createSwapChain(HWND hWnd,
	uint32_t width, uint32_t height,
	DXGI_FORMAT renderTargetFormat)
{
	mSwapChain = SwapChain::create(hWnd,
		mAdapter,
		mDevice,
		mDevice->directCommandQueue(), 
		width, height, 
		renderTargetFormat);
}