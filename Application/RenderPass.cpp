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

void RenderPass::setRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
	mRootSignature = rootSignature;
}

void RenderPass::setPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	mPipelineState = pipelineState;
}

void RenderPass::setName(const std::string& name)
{
	mName = name;
}

std::string RenderPass::getName() const
{
	return mName;
}