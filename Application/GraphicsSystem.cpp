#include "framework.h"
#include "GraphicsSystem.h"
#include "CommandQueue.h"
#include "CubeMesh.h"
#include "Adapter.h"
#include "SimpleScene.h"
#include "SimpleEffect.h"
#include "Camera.h"
#include "Frame.h"
#include "World.h"
#include "Node.h"
#include "helper.h"
#include "CommandList.h"
#include "RenderGraph.h"
#include "ShadowPass.h"
#include "BeautyPass.h"
#include "Managers.h"
#include "TextureManager.h"
#include "LightManager.h"
#include "Light.h"
#include "CameraManager.h"
#include "HeapManager.h"
#include "TextureResource.h"
#include <vector>
#include <DirectXMath.h>

GraphicsSystem::GraphicsSystem()
{
	
}

GraphicsSystem::~GraphicsSystem()
{
	delete mEffect;
	delete mCamera;
	delete mScene;

	// free RenderGraph array
	delete[] mRenderGraph;

	// free frame and frame data array
	delete[] mFrames;
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

	createSRVHeap();

	createSamplerHeap();

	createEventHandle();

	createFence();

	ComPtr<ID3D12GraphicsCommandList2> commandList = mDirectCommandQueue->acquireDXCommandList();
	
	createCamera();

	createWorld(commandList);

	createEffect();

	// finish upload mesh and wait until uploading finished
	auto fenceValue = mDirectCommandQueue->executeCommandListAndSignal(commandList);
	mDirectCommandQueue->waitForFenceValue(fenceValue);

	mGraphicsInitialized = true;

	resizeDepthBuffer(mWidth, mHeight);

	createManagers();

	createLightCamera();

	createFrames();

	createRenderGraph();
	
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
		mDirectCommandQueue->commandQueue(),
		width, height, 
		renderTargetFormat);
}

void GraphicsSystem::createDSVHeap()
{
	// 1 DSV view for depth stencil buffer
	// FrameCount DSV views are for shadow map DSV views
	mDSVHeap = mDevice->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 + FrameCount);
	
}

void GraphicsSystem::createSRVHeap()
{
	mSRVHeap = mDevice->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		FrameCount,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void GraphicsSystem::createSamplerHeap()
{
	mSamplerHeap = mDevice->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		1, 
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	D3D12_SAMPLER_DESC clampSamplerDesc = {};
	clampSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	clampSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	clampSamplerDesc.MipLODBias = 0.0f;
	clampSamplerDesc.MaxAnisotropy = 1;
	clampSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	clampSamplerDesc.BorderColor[0] = clampSamplerDesc.BorderColor[1] = clampSamplerDesc.BorderColor[2] = clampSamplerDesc.BorderColor[3] = 0;
	clampSamplerDesc.MinLOD = 0;
	clampSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

	// create point sampler for sample shdow map
	CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandle(mSamplerHeap->GetCPUDescriptorHandleForHeapStart());
	mDevice->createSampler(&clampSamplerDesc, samplerHandle);
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

void GraphicsSystem::createWorld(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	mWorld = std::make_shared<World>();

	mScene = new SimpleScene();
	
	mWorld->setScene(mScene);
	mWorld->setCamera(mCamera);

	const int meshCount = 10;

	// distribute meshes uniformally in space [-500, 500]^3
	for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex)
	{
		std::shared_ptr<Mesh> mesh = std::make_shared<CubeMesh> ();
		mesh->build(this, commandList);

		Node* node = new Node();
		node->setMesh(mesh);

		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX S, R, RX, RY, RZ, T;

		// scale
		float scale = (std::rand() * 1.0f / RAND_MAX + 1.0f) * 0.7f;

		S = DirectX::XMMatrixScaling(scale, scale, scale);

		// rotate
		float rotateX, rotateY, rotateZ;

		rotateX = std::rand() * 1.f / RAND_MAX * DirectX::XM_2PI;
		RX = DirectX::XMMatrixRotationX(rotateX);

		rotateY = std::rand() * 1.f / RAND_MAX * DirectX::XM_2PI;
		RY = DirectX::XMMatrixRotationY(rotateY);

		rotateZ = std::rand() * 1.f / RAND_MAX * DirectX::XM_2PI;

		RZ = DirectX::XMMatrixRotationZ(rotateZ);

		R = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(RX, RY), RZ);

		// translate
		float translateX, translateY, translateZ;

		translateX = std::rand() * 1.0f / RAND_MAX * 10.0f - 5.0f;
		translateY = std::rand() * 1.0f / RAND_MAX * 10.0f - 5.0f;

		translateZ = std::rand() * 1.0f / RAND_MAX * 10.0f - 5.0f;

		T = DirectX::XMMatrixTranslation(translateX, translateY, translateZ);

		node->setWorldMatrix(T * S * R);

		mScene->addNode(node);
	}

}

void GraphicsSystem::createRenderGraph()
{
	mRenderGraph = new RenderGraph[FrameCount];

	std::string baseName("ShadowSimple");

	for (int frameIndex = 0; frameIndex < FrameCount; ++frameIndex)
	{
		std::string fullName = baseName + std::to_string(frameIndex);
		mRenderGraph[frameIndex].setName(fullName);
		mRenderGraph[frameIndex].setFrameData(mFrames[frameIndex].getFrameData());
		mRenderGraph[frameIndex].createPasses();

		mFrames[frameIndex].setRenderGraph(&(mRenderGraph[frameIndex]));

	}
}

void GraphicsSystem::createEffect()
{
	mEffect = new SimpleEffect();
	mEffect->build(mDevice);
}

void GraphicsSystem::createFrames()
{
	// create Frame objects
	mFrames = new Frame[FrameCount];

	for (int frameIndex = 0; frameIndex < FrameCount; ++frameIndex)
	{
		mFrames[frameIndex].setFrameCount(FrameCount);
		mFrames[frameIndex].setFrameIndex(frameIndex);

		// setup frame data
		mFrames[frameIndex].setDirectCommandQueue(mDirectCommandQueue);
		mFrames[frameIndex].createCommandList(mDevice);
		mFrames[frameIndex].setWorld(mWorld);
		mFrames[frameIndex].setViewport(mViewport);
		mFrames[frameIndex].setScissorRect(mScissorRect);
		mFrames[frameIndex].setManagers(mManagers);
	}
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

void GraphicsSystem::createManagers()
{
	mManagers = std::make_shared<Managers>(mDevice);

	// add heaps to heapManager
	mManagers->getHeapManager()->addHeap("MainDSVHeap", mDSVHeap);

	mManagers->getHeapManager()->addHeap("MainSRVHeap", mSRVHeap);

	mManagers->getHeapManager()->addHeap("MainSamplerHeap", mSamplerHeap);

	std::shared_ptr<TextureManager> textureMan = mManagers->getTextureManager();

	// set backbuffer resource and backbuffer RTV view to texture manager
	std::string backBufferBaseName("BackBuffer");

	for (int frameIndex = 0; frameIndex < FrameCount; ++frameIndex)
	{
		std::shared_ptr<TextureResource> backBuffer = std::make_shared<TextureResource>();

		backBuffer->mResource = mSwapChain->getBackBuffer(frameIndex);
		backBuffer->mRTV = mSwapChain->getRTV(frameIndex);

		std::string backBufferFullName = backBufferBaseName + std::to_string(frameIndex);
		textureMan->addTexture(backBufferFullName, backBuffer);
	}


	// set depth stencil buffer RTV view
	std::shared_ptr<TextureResource> depthBuffer = std::make_shared<TextureResource>();
	// only depth stencil view is needed
	depthBuffer->mDSV = mDSVHeap->GetCPUDescriptorHandleForHeapStart();
	textureMan->addTexture("DepthStencil", depthBuffer);

	// create depth resource as shadow map
	CD3DX12_RESOURCE_DESC shadowTextureDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		static_cast<UINT>(1024),
		static_cast<UINT>(1024),
		1,
		1,
		DXGI_FORMAT_R32_TYPELESS,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);


	D3D12_CLEAR_VALUE optimizedClearValue = {};
	optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	optimizedClearValue.DepthStencil = { 1.0f, 0 };


	std::string shadowBaseName("shadowMap");
	// descriptor size is vendor specific, thus it must be queried
	const UINT dsvDescriptorSize = mDevice->getDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	for (int frameIndex = 0; frameIndex < FrameCount; ++frameIndex)
	{
		std::shared_ptr<TextureResource> shadowMap = std::make_shared<TextureResource>();

		// create shadow map resource
		mDevice->createCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&shadowTextureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&shadowMap->mResource));

		// create DSV view
		// offset by 1, skip depth stencil RTV view
		CD3DX12_CPU_DESCRIPTOR_HANDLE shadowDSV(mDSVHeap->GetCPUDescriptorHandleForHeapStart(), 1 + frameIndex, dsvDescriptorSize);

		D3D12_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc = {};
		shadowDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
		shadowDSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		shadowDSVDesc.Texture2D.MipSlice = 0;
		mDevice->createDepthStencilView(shadowMap->mResource.Get(), &shadowDSVDesc, shadowDSV);
		shadowMap->mDSV = shadowDSV;

		// create SRV view
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVCPUView(mSRVHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_GPU_DESCRIPTOR_HANDLE SRVGPUView(mSRVHeap->GetGPUDescriptorHandleForHeapStart());

		D3D12_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
		shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shadowSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shadowSRVDesc.Texture2D.MipLevels = 1;
		shadowSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		mDevice->createShaderResourceView(shadowMap->mResource.Get(), &shadowSRVDesc, SRVCPUView);

		shadowMap->mSRV = SRVGPUView;

		std::string shadowFullName = shadowBaseName + std::to_string(frameIndex);
		textureMan->addTexture(shadowFullName, shadowMap);
	}
}

void GraphicsSystem::createLightCamera()
{
	std::shared_ptr<Light> light = mManagers->getLightManager()->getLight("SimpleLight0");

	std::shared_ptr<Camera> camera = std::make_shared<Camera>();

	const DirectX::XMVECTOR eye = XMLoadFloat4(&light->mPosition);
	const DirectX::XMVECTOR target = DirectX::XMVectorAdd(eye, XMLoadFloat4(&light->mDirection));
	const DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	camera->viewMaxtrix(eye, target, up);

	// projection matrix.
	float aspectRatio = mWidth / static_cast<float>(mHeight);
	camera->projectionMaxtrix(aspectRatio, 0.1f, 100.0f);

	// not model matrix need to be involved
	camera->computeViewProjectionMatrix();

	mManagers->getCameraManager()->addCamera("Light0Camera", camera);
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

	modelMatrix *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 15.0f);

	mCamera->modelMaxtrix(modelMatrix);

	mCamera->computeModelViewProjectionMatrix();
}

void GraphicsSystem::render()
{
	renderWorld();
}

void GraphicsSystem::finish()
{
	mDirectCommandQueue->flush();
}

void GraphicsSystem::renderWorld()
{
	// new  render
	// get current buffer index
	UINT currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

	mFrames[currentBackBufferIndex].beginFrame();

	mFrames[currentBackBufferIndex].renderFrame();

	mFrames[currentBackBufferIndex].endFrame();

	//mFrames[currentBackBufferIndex].reset();

	mSwapChain->present();

	mFrameFenceValues[currentBackBufferIndex] = mDirectCommandQueue->signal();

	currentBackBufferIndex = mSwapChain->getCurrentBackBufferIndex();

	mDirectCommandQueue->waitForFenceValue(mFrameFenceValues[currentBackBufferIndex]);

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