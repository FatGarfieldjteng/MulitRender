#pragma once

#include <wrl/client.h>  // For Microsoft::WRL::ComPtr
#include <d3d12.h>
#include "SwapChain.h"
#include "Device.h"
#include <memory>
#include <string>

class Mesh;
class CommandQueue;
class Scene;
class Effect;
class Camera;
class Frame;
class World;
class RenderGraph;
class Managers;

namespace enki
{
	class TaskScheduler;
};

class GraphicsSystem
{
public:

	static const UINT FrameCount = 3;

	GraphicsSystem();
	~GraphicsSystem();

public:

	void initGraphicsSystem(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM);

	ComPtr<ID3D12GraphicsCommandList2> acquireCommandList();

	// create a GPU buffer.
	void updateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource,
		ID3D12Resource** pIntermediateResource,
		size_t numElements,
		size_t elementSize,
		const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	void update();

	void render();

	void finish();

protected:

	void createDevice();

	void createDirectCommandQueue();
	
	void createSwapChain(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM);

	void createDSVHeap();

	void createSRVHeap();

	void createSamplerHeap();

	void createEventHandle();

	void createFence();

	void createWorld(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void createRenderGraph();

	void createEffect();

	void createCamera();

	void createManagers();

	void createLightCamera();

	void initTasks();

	void createFrames();

	void updateCamera(double elapsedTime);

private:
	// helper functions
	// resize the depth buffer to match the size of the client area.
	void resizeDepthBuffer(int width, int height);

	void renderWorld();

private:
	UINT mWidth = 0;
	UINT mHeight = 0;

	std::shared_ptr<Adapter> mAdapter;
	std::shared_ptr<Device> mDevice;
	std::shared_ptr<SwapChain> mSwapChain;
	std::shared_ptr<CommandQueue> mDirectCommandQueue;

	std::shared_ptr<Managers> mManagers;

	// depth buffer.
	ComPtr<ID3D12Resource> mDepthBuffer;

	ComPtr<ID3D12DescriptorHeap> mDSVHeap;
	ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	ComPtr<ID3D12DescriptorHeap> mSamplerHeap;

	D3D12_VIEWPORT mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, 1.0f, 1.f);
	D3D12_RECT mScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12Fence> mFence;
	uint64_t mFenceValue = 0;

	uint64_t mFrameFenceValues[FrameCount] = {};

	HANDLE mFenceEvent = 0;

// graphics data
private:
	// scene
	std::shared_ptr<World> mWorld;
	RenderGraph* mRenderGraph = nullptr;
	Scene *mScene = nullptr;
	Effect *mEffect = nullptr;
	Camera* mCamera = nullptr;
	bool mGraphicsInitialized = false;

	double mTotalElapsedSeconds = 0.0;

	std::shared_ptr<enki::TaskScheduler> mTaskScheduler;
	uint32_t mThreads;

	Frame* mFrames = nullptr;
};