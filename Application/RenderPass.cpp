#include "framework.h"
#include "RenderPass.h"

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

void RenderPass::addInput(std::shared_ptr<TextureResource> resource, ResouceType resourceType)
{
	mInputResources.push_back(resource);
	mInputResourceType.push_back(resourceType);
}

void RenderPass::addOutput(std::shared_ptr < TextureResource> resource, ResouceType resourceType)
{
	mOutputResources.push_back(resource);
	mOutputResourceType.push_back(resourceType);
}

void RenderPass::setName(const std::string& name)
{
	mName = name;
}

std::string RenderPass::getName() const
{
	return mName;
}