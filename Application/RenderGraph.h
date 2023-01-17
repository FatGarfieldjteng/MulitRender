#pragma once
#include <memory>
#include <vector>

class RenderPass;
class GraphicsResource;

class RenderGraph
{
public:

    RenderGraph();

    ~RenderGraph();

public:
    void addRenderPass(std::shared_ptr<RenderPass*> renderPass);
    void linkRenerPass(std::shared_ptr <RenderPass*> srcPass, 
        std::shared_ptr <RenderPass*> dstPass, 
        std::vector<GraphicsResource*> resources);
    void compile();
    void execute();

private:
    void createTasks();
    std::vector< std::shared_ptr<RenderPass*> > mRenderPasses;
};
