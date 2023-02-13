#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>

class RenderPass;
class GraphicsResource;
class FrameData;

class RenderGraph
{
public:

    RenderGraph();

    ~RenderGraph();

public:
    void setName(const std::string& name);
    std::string getName() const;
    void createPasses();
    void addRenderPass(std::shared_ptr<RenderPass> renderPass);
    void linkRenerPass(std::shared_ptr <RenderPass> srcPass, 
        std::shared_ptr <RenderPass> dstPass, 
        std::vector<GraphicsResource> resources);
    void compile();
    void execute(std::shared_ptr<FrameData> frameData);

private:
    std::string mName;
    void createShadowPass();
    void createBeautyPass();
    std::map<std::string, std::shared_ptr<RenderPass>> mIDToRenderPass;
    std::vector< std::shared_ptr<RenderPass> > mRenderPasses;
};
