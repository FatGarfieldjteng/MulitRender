#include "framework.h"
#include "RenderPass.h"
#include "CommandRecorder.h"
#include "Device.h"

RenderPass::RenderPass(std::shared_ptr<Device> device)

{
	mCommandRecorder = std::make_unique<CommandRecorder>(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

RenderPass::~RenderPass()
{

}

void RenderPass::addInput(std::shared_ptr<GraphicsResource*> resource)
{
	mInputResources.push_back(resource);
}

void RenderPass::addOutput(std::shared_ptr < GraphicsResource*> resource)
{
	mOuputResources.push_back(resource);
}