#include "framework.h"
#include "RenderGraph.h"
#include "GraphicsResource.h"
#include "ShadowPass.h"
#include "BeautyPass.h"
#include "FrameData.h"
#include "Managers.h"
#include "TextureManager.h"
#include "TextureResource.h"
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

void RenderGraph::setFrameData(FrameData* frameData)
{
    mFrameData = frameData;
}

void RenderGraph::createPasses()
{
    //createShadowPass();
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

    std::string baseName("BackBuffer");

    std::string backbufferName = baseName + std::to_string(mFrameData->mFrameIndex);

    // get backbuffer
    std::shared_ptr<TextureResource> backBuffer = mFrameData->mManagers->getTextureManager()->getTexture(backbufferName);
    std::shared_ptr<TextureResource> depthStencil = mFrameData->mManagers->getTextureManager()->getTexture("DepthStencil");
    
    beautyPass->addOutput(backBuffer, RenderPass::ResourceType_BackBuffer);
    beautyPass->addOutput(depthStencil, RenderPass::ResourceType_DepthStencil);
    addRenderPass(beautyPass);
}

void RenderGraph::addRenderPass(std::shared_ptr<RenderPass> renderPass)
{
    mRenderPasses.push_back(renderPass);
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

void RenderGraph::execute()
{
    // Run passes one by one
    for (std::shared_ptr <RenderPass> renderPass: mRenderPasses)
    {
        renderPass->render(mFrameData);
    }
}