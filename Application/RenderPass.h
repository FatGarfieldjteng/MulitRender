#pragma once

#include <memory>
#include <vector>

class GraphicsResource;
class CommandRecorder;
class RenderTask;
class Device;

namespace enki {
    class TaskScheduler;
}

class RenderPass
{
public:

    RenderPass(std::shared_ptr<Device> device);

    ~RenderPass();

public:
    void setRenderData();
    void addInput(std::shared_ptr<GraphicsResource*> resource);
    void addOutput(std::shared_ptr < GraphicsResource*> resource);

public:
    virtual void buildTasks() = 0;

    virtual void preprocess() {};
    virtual void execute() = 0;
    virtual void postprocess() {};

protected:
    std::vector < std::shared_ptr<GraphicsResource*> > mInputResources;
    std::vector < std::shared_ptr<GraphicsResource*> > mOuputResources;
    std::unique_ptr< CommandRecorder> mCommandRecorder;

    RenderTask* mRenerTask = nullptr;

    std::shared_ptr<enki::TaskScheduler> mTaskScheduler;
};
