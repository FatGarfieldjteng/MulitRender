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

void GraphicsSystem::createEventHandle()
{
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(mFenceEvent && "Failed to create fence event.");
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
	waitForFenceValue();
}


void GraphicsSystem::Update()
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

void GraphicsSystem::Render()
{
	auto commandAllocator = mCommandAllocators[mCurrentBackBufferIndex];
	auto backBuffer = mSwapChain->backBuffer(mCurrentBackBufferIndex);

	commandAllocator->Reset();
	mCommandList->Reset(commandAllocator.Get(), nullptr);

	// Clear the render target.
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		mCommandList->ResourceBarrier(1, &barrier);


		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrentBackBufferIndex, mRTVDescriptorSize);

		mCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}


	// Present
	{
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		mCommandList->ResourceBarrier(1, &barrier);

		ThrowIfFailed(mCommandList->Close());

		ID3D12CommandList* const commandLists[] = {
			mCommandList.Get()
		};
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);


		mSwapChain->present();

		mFrameFenceValues[mCurrentBackBufferIndex] = Signal(g_FenceValue);

		mCurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

		waitForFenceValue(mFrameFenceValues[mCurrentBackBufferIndex]);
	}
}