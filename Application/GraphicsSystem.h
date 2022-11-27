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

protected:

	void createDevice();

	void createDirectCommandQueue();
	
	void createSwapChain(HWND hWnd,
		uint32_t width, uint32_t height,
		DXGI_FORMAT renderTargetFormat = DXGI_FORMAT_R10G10B10A2_UNORM);

	void createDSVHeap();

	void createCommandAllocators();

	void createCommandList();

	void createEventHandle();

	void createFence();

	uint64_t signal();

	void waitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max());

	void flush(uint64_t& fenceValue);

	void createScene(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void createEffect();

	void createCamera();

private:
	// helper functions
	// transition a resource
	void transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
		ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState, 
		D3D12_RESOURCE_STATES afterState);

	// Clear a render target view.
	void clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv,
		FLOAT* clearColor);

	// clear the depth of a depth-stencil view.
	void clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
		FLOAT depth = 1.0f);

	// resize the depth buffer to match the size of the client area.
	void resizeDepthBuffer(int width, int height);


private:
	std::shared_ptr<Adapter> mAdapter;
	std::shared_ptr<Device> mDevice;
	std::shared_ptr<SwapChain> mSwapChain;
	std::shared_ptr<CommandQueue> mDirectCommandQueue;

	// depth buffer.
	ComPtr<ID3D12Resource> mDepthBuffer;

	ComPtr<ID3D12DescriptorHeap> mDSVHeap;

	

	D3D12_VIEWPORT mViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, 1.0f, 1.f);
	D3D12_RECT mScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

	ComPtr<ID3D12CommandAllocator> mCommandAllocators[BufferCount];
	ComPtr<ID3D12GraphicsCommandList> mCommandList;

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12Fence> mFence;
	uint64_t mFenceValue = 0;

	uint64_t mFrameFenceValues[BufferCount] = {};

	HANDLE mFenceEvent = 0;

// graphics data
private:
	// scene
	Scene *mScene = nullptr;
	Effect *mEffect = nullptr;
	Camera* mCamera = nullptr;
	bool mGraphicsInitialized = false;
};