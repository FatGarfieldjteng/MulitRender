#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>

class RenderPass;
class GraphicsResource;
class FrameData;
class Managers;

class RenderGraph
{
public:

    RenderGraph();

    ~RenderGraph();

public:
    void setName(const std::string& name);
    std::string getName() const;
    void setFrameData(FrameData* frameData);
    void createPasses();
    void addRenderPass(std::shared_ptr<RenderPass> renderPass);
    void linkRenerPass(std::shared_ptr <RenderPass> srcPass, 
        std::shared_ptr <RenderPass> dstPass, 
        std::vector<GraphicsResource> resources);
    void compile();
    void execute();

private:
    void createShadowPass();
    void createBeautyPass();

private:
    std::string mName;
    FrameData* mFrameData = nullptr;
    std::map<std::string, std::shared_ptr<RenderPass>> mIDToRenderPass;
    std::vector< std::shared_ptr<RenderPass> > mRenderPasses;
};
