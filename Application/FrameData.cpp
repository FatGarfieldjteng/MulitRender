#include "framework.h"
#include "FrameData.h"
#include "Device.h"
#include "CommandList.h"
#include "World.h"
#include "Scene.h"
#include "Node.h"
#include "Mesh.h"
#include "Camera.h"
#include "CommandQueue.h"
#include "RenderGraph.h"
#include "RenderPass.h"
#include "Managers.h"

#include "../ThirdParty/enkiTS/src/TaskScheduler.h"

FrameData::FrameData()
{

}

FrameData::~FrameData()
{

}

void FrameData::setFrameIndex(unsigned int frameIndex)
{
	mFrameIndex = frameIndex;
}

void FrameData::createCommandList(std::shared_ptr<Device> device)
{
	// frame begin command list
	mclBeginFrame = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandLists.push_back(mclBeginFrame.get());

	// frame end command list
	mclEndFrame = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandLists.push_back(mclEndFrame.get());

	// render command list
	mclRender = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandLists.push_back(mclRender.get());

	mNumTasks = mTaskScheduler->GetNumTaskThreads();
	mCompoundRenderCommandList = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT, mNumTasks);
}

void FrameData::setWorld(std::shared_ptr<World> world)
{
	mWorld = world;
}

void FrameData::setRenderGraph(RenderGraph* renderGraph)
{
	mRenderGraph = renderGraph;
}

void FrameData::setManagers(std::shared_ptr<Managers> managers)
{
	mManagers = managers;
}

void FrameData::setDirectCommandQueue(std::shared_ptr<CommandQueue> directCommandQueue)
{
	mDirectCommandQueue = directCommandQueue;
}

void FrameData::setViewport(const D3D12_VIEWPORT& viewport)
{
	mViewport = viewport;
}

void FrameData::setScissorRect(const D3D12_RECT& scissorRect)
{
	mScissorRect = scissorRect;
}

void FrameData::setTaskScheduler(std::shared_ptr<enki::TaskScheduler> taskScheduler)
{
	mTaskScheduler = taskScheduler;
}

void FrameData::beginFrame()
{

}

void FrameData::renderFrame()
{
	mRenderGraph->execute();
}

void FrameData::endFrame()
{

}