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
    void linkRenderPass(const std::string& srcPass, 
        const std::string& dstPass,
        const std::string& resource);
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
