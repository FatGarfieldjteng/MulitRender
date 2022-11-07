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

void GraphicsSystem::initGraphicsSystem(HWND hWnd,
	uint32_t width, uint32_t height,
	DXGI_FORMAT renderTargetFormat)
{
	createDevice();

	createDirectCommandQueue();

	createSwapChain(hWnd,
		width,
		height,
		renderTargetFormat);

	createCommandAllocators();

	createCommandList();

	createEventHandle();

	createFence();
}

void GraphicsSystem::createDevice()
{
	mAdapter = Adapter::Create();

	mDevice = Device::create(mAdapter);
	 
}

void GraphicsSystem::createDirectCommandQueue()
{
	mCommandQueue = mDevice->createDirectCommandQueue();
}

void GraphicsSystem::createSwapChain(HWND hWnd,
	uint32_t width, uint32_t height,
	DXGI_FORMAT renderTargetFormat)
{
	mSwapChain = SwapChain::create(hWnd,
		mAdapter,
		mDevice,
		mCommandQueue,
		width, height, 
		renderTargetFormat);
}

void GraphicsSystem::createCommandAllocators()
{
	for (int i = 0; i < BufferCount; ++i)
	{
		mCommandAllocators[i] = mDevice->createCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
}

void GraphicsSystem::createCommandList()
{
	mCommandList = mDevice->createCommandList(mCommandAllocators[0], D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void GraphicsSystem::createEventHandle()
{
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(mFenceEvent && "Failed to create fence event.");
}

void GraphicsSystem::createFence()
{
	mFence = mDevice->createFence();
}

uint64_t GraphicsSystem::signal()
{
	uint64_t fenceValueForSignal = ++mFenceValue;
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

void GraphicsSystem::waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration)
{
	if (mFence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
		::WaitForSingleObject(mFenceEvent, static_cast<DWORD>(duration.count()));
	}
}

void GraphicsSystem::flush(uint64_t& fenceValue)
{
	uint64_t fenceValueForSignal = signal();
	waitForFenceValue(fenceValueForSignal);
}


void GraphicsSystem::update()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0;
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 1.0)
	{
		char buffer[500];
		auto fps = frameCounter / elapsedSeconds;

		std::string strFPS = std::to_string(fps);
		OutputDebugString("FPS: ");
		OutputDebugString(strFPS.c_str());
		OutputDebugString("\n");

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}
}

void GraphicsSystem::render()
{
	UINT currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

	auto commandAllocator = mCommandAllocators[currentBackBufferIndex];

	commandAllocator->Reset();
	mCommandList->Reset(commandAllocator.Get(), nullptr);

	auto backBuffer = mSwapChain->getCurrentBackBuffer();

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		backBuffer.Get(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	mCommandList->ResourceBarrier(1, &barrier);


	mSwapChain->clearRTV(mCommandList);
	
	// present
	{
		
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &barrier);

		ThrowIfFailed(mCommandList->Close());

		// execute
		ID3D12CommandList* const commandLists[] = {
			mCommandList.Get()
		};
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);


		mSwapChain->present();

		mFrameFenceValues[currentBackBufferIndex] = signal();

		currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

		waitForFenceValue(mFrameFenceValues[currentBackBufferIndex]);
	}
}