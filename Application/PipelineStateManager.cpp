#include "framework.h"
#include "PipelineStateManager.h"

PipelineStateManager::PipelineStateManager()
{

}

PipelineStateManager::~PipelineStateManager()
{

}

void PipelineStateManager::addPipelineState(const std::string& ID, PipelineState* pipelineState)
{
	mStringToPipelineState[ID] = pipelineState;
}

PipelineState* PipelineStateManager::pipelineState(const std::string& ID)
{
	std::map<std::string, PipelineState*>::iterator it = mStringToPipelineState.find(ID);

	if (it != mStringToPipelineState.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}