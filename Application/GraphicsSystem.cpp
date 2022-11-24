#include "framework.h"
#include "GraphicsSystem.h"
#include "CommandQueue.h"
#include "CubeMesh.h"
#include "Adapter.h"
#include "SimpleScene.h"
#include "SimpleEffect.h"
#include "helper.h"

GraphicsSystem::GraphicsSystem()
{

}

GraphicsSystem::~GraphicsSystem()
{
	delete mScene;
	delete mEffect;
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

	createDSVHeap();

	createCommandAllocators();

	createCommandList();

	createEventHandle();

	createFence();

	createScene();

	createEffect();

	createRootSignature();

	auto fenceValue = commandQueue->ExecuteCommandList(commandList);
	commandQueue->WaitForFenceValue(fenceValue);

	mGraphicsInitialized = true;
}

void GraphicsSystem::createDevice()
{
	mAdapter = Adapter::Create();

	mDevice = Device::create(mAdapter);
}

void GraphicsSystem::createDirectCommandQueue()
{
	mDirectCommandQueue = CommandQueue::create(mDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);

	mCommandQueue = mDevice->createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
}

ComPtr<ID3D12GraphicsCommandList2> GraphicsSystem::acquireCommandList()
{
	return mDirectCommandQueue->acquireCommandList();
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

void GraphicsSystem::createDSVHeap()
{
	mDSVHeap = mDevice->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
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

void GraphicsSystem::createScene()
{
	mScene = new SimpleScene(this);
}

void GraphicsSystem::createEffect()
{
	mEffect = new SimpleEffect();
}

void GraphicsSystem::createRootSignature()
{
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// version is hard coded, 
	//in real scenario, featureData.HighestVersion could be 1_0 if hardware does not suppot 1_1
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	// only input assembler stage needs root signature
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
	rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	// no sampler
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// serialize root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	mDevice->CreateRootSignature(rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

void GraphicsSystem::createPipelineStateObject()
{
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	pipelineStateStream.pRootSignature = mRootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	ThrowIfFailed(mDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState)));
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

void GraphicsSystem::transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
	ComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES beforeState,
	D3D12_RESOURCE_STATES afterState)
{

}

// Clear a render target view.
void GraphicsSystem::clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, 
	FLOAT* clearColor)
{

}

// clear the depth of a depth-stencil view.
void GraphicsSystem::clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
	FLOAT depth)
{

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
	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(pDestinationResource));

	// create an committed resource in CPU side and copy data from bufferData to the resouce.
	if (bufferData)
	{
		mDevice->CreateCommittedResource(
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

}