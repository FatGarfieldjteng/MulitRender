#include "framework.h"
#include "RenderGraph.h"
#include "GraphicsResource.h"
#include "ShadowPass.h"
#include "BeautyPass.h"
#include "FrameData.h"
#include "Managers.h"
#include "TextureManager.h"
#include "RootSignatureManager.h"
#include "PipelineStateManager.h"
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
    createShadowPass();
    createBeautyPass();

    std::string shadowBaseName("shadowMap");
    std::string shadowFullName = shadowBaseName + std::to_string(mFrameData->mFrameIndex);

    linkRenderPass("ShadowPass", "BeautyPass", shadowFullName);
}

void RenderGraph::createShadowPass()
{
    std::shared_ptr<RenderPass> shadowPass = std::make_shared<ShadowPass>();
    shadowPass->setName("ShadowPass");

    std::string shadowBaseName("shadowMap");

    std::string shadowFullName = shadowBaseName + std::to_string(mFrameData->mFrameIndex);

    shadowPass->addOutput(shadowFullName, 
        mFrameData->mManagers->getTextureManager()->getTexture(shadowFullName), 
        RenderPass::ResourceType_RenderTarget);

    shadowPass->setRootSignature(mFrameData->mManagers->getRootSignatureManager()->getRootSignature("ShadowRootSignature"));

    shadowPass->setPipelineState(mFrameData->mManagers->getPipelineStateManager()->getPipelineState("ShadowPipelineState"));

    addRenderPass(shadowPass);
}

void RenderGraph::createBeautyPass()
{
    std::shared_ptr<RenderPass> beautyPass = std::make_shared<BeautyPass>();
    beautyPass->setName("BeautyPass");

    // shadow map as input
    std::string shadowBaseName("shadowMap");

    std::string shadowFullName = shadowBaseName + std::to_string(mFrameData->mFrameIndex);
    beautyPass->addInput(shadowFullName,
        mFrameData->mManagers->getTextureManager()->getTexture(shadowFullName), 
        RenderPass::ResourceType_ShaderResource);

    // backbuffer and depth stencil buffer as output
    std::string backBufferBaseName("BackBuffer");

    std::string backBufferFullName = backBufferBaseName + std::to_string(mFrameData->mFrameIndex);
    beautyPass->addOutput(backBufferFullName,
        mFrameData->mManagers->getTextureManager()->getTexture(backBufferFullName), 
        RenderPass::ResourceType_BackBuffer);
    beautyPass->addOutput("DepthStencil",
        mFrameData->mManagers->getTextureManager()->getTexture("DepthStencil"), 
        RenderPass::ResourceType_DepthStencil);

    beautyPass->setRootSignature(mFrameData->mManagers->getRootSignatureManager()->getRootSignature("SimpleRootSignature"));

    beautyPass->setPipelineState(mFrameData->mManagers->getPipelineStateManager()->getPipelineState("SimplePipelineState"));

    addRenderPass(beautyPass);
}

void RenderGraph::addRenderPass(std::shared_ptr<RenderPass> renderPass)
{
    mRenderPasses.push_back(renderPass);
}

void RenderGraph::linkRenderPass(const std::string& srcPass,
    const std::string& dstPass,
    const std::string& resource)
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