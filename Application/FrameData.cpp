#include "framework.h"
#include "FrameData.h"
#include "Device.h"
#include "CommandList.h"
#include "World.h"

FrameData::FrameData()
{

}

FrameData::~FrameData()
{

}

void FrameData::createCommandList(std::shared_ptr<Device> device)
{
	mCommandList = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mDX12CommandLists.push_back(mCommandList->commandList().Get());
	mCommandLists.push_back(mCommandList.get());

	mRenderCommandList = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mDX12CommandLists.push_back(mRenderCommandList->commandList().Get());
	mCommandLists.push_back(mRenderCommandList.get());

}

void FrameData::setWorld(std::shared_ptr<World> world)
{
	mWorld = world;
}

void FrameData::setViewport(const D3D12_VIEWPORT& viewport)
{
	mViewport = viewport;
}

void FrameData::setScissorRect(const D3D12_RECT& scissorRect)
{
	mScissorRect = scissorRect;
}

void FrameData::setBackBufferResource(ComPtr<ID3D12Resource> backBuffer)
{
	mBackBuffer = backBuffer;
}

void FrameData::setBackBufferView(const D3D12_CPU_DESCRIPTOR_HANDLE& backBufferView)
{
	mBackBufferView = backBufferView;
}

void FrameData::setDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilView)
{
	mDepthBufferView = depthStencilView;
}

void FrameData::setGraphicsRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
	mRootSignature = rootSignature;
}

void FrameData::setPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	mPipelineState = pipelineState;
}

void FrameData::beginFrame()
{

	mCommandList->transitionResource(mBackBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	mCommandList->clearRTV(clearColor, mBackBufferView);
}

void FrameData::renderFrame()
{
	
}

void FrameData::endFrame()
{
	mCommandList->transitionResource(mBackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
}

void FrameData::reset()
{
	for (CommandList* commandlist : mCommandLists)
	{
		commandlist->reset();
	}
}


std::unique_ptr<CommandList>& FrameData::getCommandList()
{
	return mCommandList;
}

std::vector<ID3D12CommandList*>& FrameData::getCommandLists()
{
	return mDX12CommandLists;
}
