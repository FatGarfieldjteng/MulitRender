#include "framework.h"
#include "GraphicsSystem.h"
#include "CommandQueue.h"
#include "CubeMesh.h"
#include "Adapter.h"
#include "SimpleScene.h"
#include "SimpleEffect.h"
#include "Camera.h"
#include "helper.h"
#include <DirectXMath.h>

GraphicsSystem::GraphicsSystem()
{

}

GraphicsSystem::~GraphicsSystem()
{
	delete mScene;
	delete mEffect;
	delete mCamera;
}

void GraphicsSystem::initGraphicsSystem(HWND hWnd,
	uint32_t width, uint32_t height,
	DXGI_FORMAT renderTargetFormat)
{
	mWidth = width;
	mHeight = height;

	mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
		static_cast<float>(mWidth), static_cast<float>(mHeight));

	createDevice();

	createDirectCommandQueue();

	createSwapChain(hWnd,
		width,
		height,
		renderTargetFormat);

	createDSVHeap();

#ifdef RAW_MODE
	createCommandAllocators();

	createCommandList();
#endif

	createEventHandle();

	createFence();

	ComPtr<ID3D12GraphicsCommandList2> commandList = mDirectCommandQueue->acquireDXCommandList();

	createScene(commandList);

	createCamera();

	createEffect();

	// finish upload mesh and wait until uploading finished
	auto fenceValue = mDirectCommandQueue->executeCommandList(commandList);
	mDirectCommandQueue->waitForFenceValue(fenceValue);

	//// scene uploaded to GPU, then release intermediate buffer
	mScene->endBuild();

	mGraphicsInitialized = true;

	resizeDepthBuffer(mWidth, mHeight);
	
}

void GraphicsSystem::createDevice()
{
	mAdapter = Adapter::Create();

	mDevice = Device::create(mAdapter);

	mDevice->createCommandQueues();

	mDevice->createCPUDescriptorAllocators();
}

void GraphicsSystem::createDirectCommandQueue()
{
	mDirectCommandQueue = std::make_shared<CommandQueue>(mDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);

	mCommandQueue = mDevice->createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
}

ComPtr<ID3D12GraphicsCommandList2> GraphicsSystem::acquireCommandList()
{
	return mDirectCommandQueue->acquireDXCommandList();
}

void GraphicsSystem::createSwapChain(HWND hWnd,
	uint32_t width, uint32_t height,
	DXGI_FORMAT renderTargetFormat)
{
	mSwapChain = SwapChain::create(hWnd,
		mAdapter,
		mDevice,
#ifdef RAW_MODE
		mCommandQueue,
#else
		mDirectCommandQueue->commandQueue(),
#endif
		width, height, 
		renderTargetFormat);
}

void GraphicsSystem::createDSVHeap()
{
	mDSVHeap = mDevice->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
}
#ifdef RAW_MODE
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
#endif
void GraphicsSystem::createEventHandle()
{
	mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(mFenceEvent && "Failed to create fence event.");
}

void GraphicsSystem::createFence()
{
	mFence = mDevice->createFence();
}

#ifdef RAW_MODE
uint64_t GraphicsSystem::signal()
{
	uint64_t fenceValueForSignal = ++mFenceValue;
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

void GraphicsSystem::flush()
{
	uint64_t fenceValueForSignal = signal();
	waitForFenceValue(fenceValueForSignal);
}

void GraphicsSystem::waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration)
{
	if (mFence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent)); 
		::WaitForSingleObject(mFenceEvent, static_cast<DWORD>(duration.count()));
	}
}

#endif

void GraphicsSystem::createScene(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	mScene = new SimpleScene();
	mScene->build(this, commandList);
}

void GraphicsSystem::createEffect()
{
	mEffect = new SimpleEffect();
	mEffect->build(mDevice);
}

void GraphicsSystem::createCamera()
{
	mCamera = new Camera();

	// view matrix.
	const DirectX::XMVECTOR eye = DirectX::XMVectorSet(0, 0, -10, 1);
	const DirectX::XMVECTOR target = DirectX::XMVectorSet(0, 0, 0, 1);
	const DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
	mCamera->viewMaxtrix(eye, target, up);

	// projection matrix.
	float aspectRatio = mWidth / static_cast<float>(mHeight);
	mCamera->projectionMaxtrix(aspectRatio, 0.1f, 100.0f);
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
	mTotalElapsedSeconds += deltaTime.count() * 1e-9;

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


	updateCamera(mTotalElapsedSeconds);
}

void GraphicsSystem::updateCamera(double elapsedTime)
{
	// model matrix.
	float angle = static_cast<float>(elapsedTime * 90.0);
	const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

	mCamera->modelMaxtrix(modelMatrix);

	mCamera->computeModelViewProjectionMatrix();
}

void GraphicsSystem::render()
{
	//clearScreen();
	renderCube();
}

void GraphicsSystem::finish()
{
	mDirectCommandQueue->flush();
}

#ifdef RAW_MODE
void GraphicsSystem::clearScreen()
{
	UINT currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

	auto commandAllocator = mCommandAllocators[currentBackBufferIndex];

	commandAllocator->Reset();
	mCommandList->Reset(commandAllocator.Get(), nullptr);

	auto backBuffer = mSwapChain->getCurrentBackBuffer();

	transitionResource(mCommandList,
		backBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	mSwapChain->clearRTV(mCommandList);

	// present
	{

		transitionResource(mCommandList,
			backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);

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

#else
void GraphicsSystem::clearScreen()
{
	UINT currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();
	auto backBuffer = mSwapChain->getCurrentBackBuffer();

	ComPtr<ID3D12GraphicsCommandList2> commandList = mDirectCommandQueue->acquireDXCommandList();

	transitionResource(commandList,
		backBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	mSwapChain->clearRTV(commandList);

	// present
	{

		transitionResource(commandList,
			backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);

		// execute
		mFrameFenceValues[currentBackBufferIndex] = mDirectCommandQueue->executeCommandList(commandList);


		mSwapChain->present();

		currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

		mDirectCommandQueue->waitForFenceValue(mFrameFenceValues[currentBackBufferIndex]);
	}
}

#endif

void GraphicsSystem::renderCube()
{
	UINT currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();
	ComPtr<ID3D12Resource> backBuffer = mSwapChain->getCurrentBackBuffer();

	ComPtr<ID3D12GraphicsCommandList2> commandList = mDirectCommandQueue->acquireDXCommandList();
	
	transitionResource(commandList,
		backBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	mSwapChain->clearRTV(commandList);

	if (mGraphicsInitialized)
	{

		auto dsv = mDSVHeap->GetCPUDescriptorHandleForHeapStart();
		clearDepth(commandList, dsv);

		commandList->SetPipelineState(mEffect->mPipelineState.Get());
		commandList->SetGraphicsRootSignature(mEffect->mRootSignature.Get());

		commandList->RSSetViewports(1, &mViewport);
		commandList->RSSetScissorRects(1, &mScissorRect);
		
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = mSwapChain->getCurrentRTV();
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		// Update the MVP matrix
		DirectX::XMMATRIX mvpMatrix = mCamera->modelViewProjectionMatrix();
		commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &mvpMatrix, 0);

		size_t meshCount = mScene->mesheCount();

		for (size_t i = 0; i < meshCount; ++i)
		{
			Mesh* mesh = mScene->mesh(i);
			commandList->IASetVertexBuffers(0, 1, &(mesh->mVertexBuffer.mVertexBufferView));
			commandList->IASetIndexBuffer(&(mesh->mIndexBuffer.mIndexBufferView));
			commandList->DrawIndexedInstanced(mesh->mIndexCount, 1, 0, 0, 0);
		}

	}

	// present
	{
		transitionResource(commandList,
			backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);

		mDirectCommandQueue->executeCommandList(commandList);

		mSwapChain->present();

		mFrameFenceValues[currentBackBufferIndex] = mDirectCommandQueue->signal();

		currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

		mDirectCommandQueue->waitForFenceValue(mFrameFenceValues[currentBackBufferIndex]);
	}
}

void GraphicsSystem::transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES beforeState,
	D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource.Get(),
		beforeState, afterState);

	commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target view.
void GraphicsSystem::clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, 
	FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

// clear the depth of a depth-stencil view.
void GraphicsSystem::clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
	FLOAT depth)
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

// create a GPU buffer.
void GraphicsSystem::updateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** pDestinationResource, 
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize, 
	const void* bufferData,
	D3D12_RESOURCE_FLAGS flags)
{
	size_t bufferSize = numElements * elementSize;

	// create a committed resource in GPU side.
	mDevice->createCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(pDestinationResource));

	// create an committed resource in CPU side and copy data from bufferData to the resouce.
	if (bufferData)
	{
		mDevice->createCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediateResource));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		// upload buffer data from CPU resource to GPU resource
		UpdateSubresources(commandList.Get(),
			*pDestinationResource, *pIntermediateResource,
			0, 0, 1, &subresourceData);
	}
	
}

// resize the depth buffer to match the size of the client area.
void GraphicsSystem::resizeDepthBuffer(int width, int height)
{
	if (mGraphicsInitialized)
	{
		// the depth stencil buffer can be resized only without write operation
		mDirectCommandQueue->flush();

		width = std::max(1, width);
		height = std::max(1, height);

		// create a depth buffer.
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		mDevice->createCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
				1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&mDepthBuffer)
		);

		// Update the depth-stencil view.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		mDevice->createDepthStencilView(mDepthBuffer.Get(), &dsv,
			mDSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}