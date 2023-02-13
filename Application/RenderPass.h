#pragma once

#include <memory>
#include <vector>
#include <string>

class GraphicsResource;
class CommandList;
class RenderTask;
class Device;
class FrameData;

namespace enki {
    class TaskScheduler;
}

class RenderPass
{
public:

    RenderPass();

    ~RenderPass();

public:
    void addInput(std::shared_ptr<GraphicsResource*> resource);
    void addOutput(std::shared_ptr < GraphicsResource*> resource);

public:
    void setName(const std::string& name);
    std::string getName() const;

    virtual void render(std::shared_ptr<FrameData> frameData) = 0;

protected:

    std::string mName;
    std::vector < std::shared_ptr<GraphicsResource*> > mInputResources;
    std::vector < std::shared_ptr<GraphicsResource*> > mOuputResources;

    RenderTask* mRenerTask = nullptr;

    std::shared_ptr<enki::TaskScheduler> mTaskScheduler;
};
