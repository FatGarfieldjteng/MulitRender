#include "framework.h"
#include "Device.h"
#include "Frame.h"
#include "FrameData.h"
#include "CommandList.h"
#include "World.h"
#include "RenderGraph.h"
#include "Managers.h"

Frame::Frame()
{
	mFrameData = new FrameData();
}

Frame::~Frame()
{
	delete mFrameData;
}

void Frame::setFrameCount(unsigned int frameCount)
{
	mFrameCount = frameCount;
}

void Frame::setFrameIndex(unsigned int frameIndex)
{
	mFrameIndex = frameIndex;
	mFrameData->setFrameIndex(frameIndex);
}

unsigned int Frame::getFrameIndex()
{
	return mFrameIndex;
}

void Frame::createCommandList(std::shared_ptr<Device> device)
{
	mFrameData->createCommandList(device);
}

void Frame::setDirectCommandQueue(std::shared_ptr<CommandQueue> directCommandQueue)
{
	mFrameData->setDirectCommandQueue(directCommandQueue);
}

void Frame::setWorld(std::shared_ptr<World> world)
{
	mFrameData->setWorld(world);
}

void Frame::setRenderGraph(RenderGraph* renderGraph)
{
	mFrameData->setRenderGraph(renderGraph);
}

void Frame::setManagers(std::shared_ptr<Managers> managers)
{
	mFrameData->setManagers(managers);
}

void Frame::setViewport(const D3D12_VIEWPORT& viewport)
{
	mFrameData->setViewport(viewport);
}

void Frame::setScissorRect(const D3D12_RECT& scissorRect)
{
	mFrameData->setScissorRect(scissorRect);
}

void Frame::setBackBufferResource(ComPtr<ID3D12Resource> backBuffer)
{
	mFrameData->setBackBufferResource(backBuffer);
}

void Frame::setBackBufferView(const D3D12_CPU_DESCRIPTOR_HANDLE& backBufferView)
{
	mFrameData->setBackBufferView(backBufferView);
}

void Frame::setDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilView)
{
	mFrameData->setDepthStencilView(depthStencilView);
}

void Frame::setGraphicsRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
	mFrameData->setGraphicsRootSignature(rootSignature);
}

void Frame::setPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	mFrameData->setPipelineState(pipelineState);
}

FrameData* Frame::getFrameData()
{
	return mFrameData;
}

void Frame::beginFrame()
{
	//mFrameData->beginFrame();
}

void Frame::renderFrame()
{
	//mFrameData->renderFrame();
	mFrameData->renderFrameA();
}

uint64_t Frame::endFrame()
{
	return 0;
	//return mFrameData->endFrame();
}

void Frame::reset()
{
	mFrameData->reset();
}