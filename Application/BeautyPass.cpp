#include "framework.h"
#include "BeautyPass.h"
#include "Device.h"
#include "FrameData.h"
#include "CommandList.h"
#include "CommandQueue.h"

BeautyPass::BeautyPass()
{
}

BeautyPass::~BeautyPass()
{

}

void BeautyPass::render(std::shared_ptr<FrameData> frameData)
{
	frameData->mclBeginFrame->transitionResource(frameData->mBackBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	frameData->mclBeginFrame->clearRTV(clearColor, frameData->mBackBufferView);

	//global settings
	frameData->mclBeginFrame->clearDepth(frameData->mDepthBufferView);

	// here, mclBeginFrame must be executed to ensure the order
	frameData->mDirectCommandQueue->executeCommandList(frameData->mclBeginFrame->commandList().Get());
}