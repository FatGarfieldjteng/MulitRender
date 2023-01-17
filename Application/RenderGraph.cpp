#include "framework.h"
#include "RenderGraph.h"
#include "GraphicsResource.h"
#include <TaskScheduler.h>

RenderGraph::RenderGraph()

{

}

RenderGraph::~RenderGraph()
{

}

void RenderGraph::addRenderPass(std::shared_ptr<RenderPass*> renderPass)
{
	mRenderPasses.push_back(renderPass);
}

void RenderGraph::linkRenerPass(std::shared_ptr <RenderPass*> srcPass,
    std::shared_ptr <RenderPass*> dstPass,
    std::vector<GraphicsResource*> resources)
{

}

void RenderGraph::compile()
{
    // step1. 

    // step2. create execution node


    // step3. 
}

void RenderGraph::execute()
{

}

void RenderGraph::createTasks()
{

}