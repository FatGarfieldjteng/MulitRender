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

FrameData* Frame::getFrameData()
{
	return mFrameData;
}

void Frame::renderFrame()
{
	mFrameData->renderFrame();
}

void Frame::reset()
{
	mFrameData->reset();
}