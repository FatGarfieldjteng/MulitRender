#include "framework.h"
#include "RenderPass.h"
#include "CommandRecorder.h"
#include "Device.h"
#include "CommandList.h"

RenderPass::RenderPass()
{

}

RenderPass::~RenderPass()
{
	if (mRenerTask)
	{
		delete mRenerTask;
	}
}

void RenderPass::addInput(std::shared_ptr<GraphicsResource*> resource)
{
	mInputResources.push_back(resource);
}

void RenderPass::addOutput(std::shared_ptr < GraphicsResource*> resource)
{
	mOuputResources.push_back(resource);
}

void RenderPass::setName(const std::string& name)
{
	mName = name;
}

std::string RenderPass::getName() const
{
	return mName;
}