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

void RenderPass::addInput(const std::string& name, 
	std::shared_ptr<TextureResource> resource, 
	ResouceType type)
{
	InOutReource inResource;
	inResource.name = name;
	inResource.resource = resource;
	inResource.type = type;
	
	mInputResources.push_back(inResource);
}

void RenderPass::addOutput(const std::string& name, 
	std::shared_ptr < TextureResource> resource, 
	ResouceType type)
{
	InOutReource outResource;
	outResource.name = name;
	outResource.resource = resource;
	outResource.type = type;
	
	mOutputResources.push_back(outResource);
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