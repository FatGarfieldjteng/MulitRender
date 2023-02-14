#include "framework.h"
#include "RenderGraph.h"
#include "GraphicsResource.h"
#include "ShadowPass.h"
#include "BeautyPass.h"
#include "FrameData.h"
#include "Managers.h"
#include "TextureManager.h"
#include <TaskScheduler.h>

RenderGraph::RenderGraph()
{

}

RenderGraph::~RenderGraph()
{
    
}

void RenderGraph::setName(const std::string& name)
{
    mName = name;
}

std::string RenderGraph::getName() const
{
    return mName;
}

void RenderGraph::createPasses()
{
    createShadowPass();
    createBeautyPass();
}

void RenderGraph::createShadowPass()
{
    std::shared_ptr<RenderPass> shadowPass = std::make_shared<ShadowPass>();
    shadowPass->setName("ShadowPass");
    addRenderPass(shadowPass);
}

void RenderGraph::createBeautyPass()
{
    std::shared_ptr<RenderPass> beautyPass = std::make_shared<BeautyPass>();
    beautyPass->setName("BeautyPass");
    beautyPass->addInput();
    beautyPass->addOutput();
    addRenderPass(beautyPass);
}

void RenderGraph::addRenderPass(std::shared_ptr<RenderPass> renderPass)
{
    mIDToRenderPass[renderPass->getName()] = renderPass;
}

void RenderGraph::linkRenerPass(std::shared_ptr <RenderPass> srcPass,
    std::shared_ptr <RenderPass> dstPass,
    std::vector<GraphicsResource> resources)
{
    //linkRenerPass(mIDToRenderPass["ShadowPass"], mIDToRenderPass["BeautyPass"], );
}

void RenderGraph::compile()
{
   // hack the compiling process
    mRenderPasses.push_back(mIDToRenderPass["ShadowPass"]);
    mRenderPasses.push_back(mIDToRenderPass["BeautyPass"]);
}

void RenderGraph::execute(std::shared_ptr<FrameData> frameData)
{
    // Run passes one by one
    for (std::shared_ptr <RenderPass> renderPass: mRenderPasses)
    {
        renderPass->render(frameData);
    }
}