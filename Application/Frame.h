#pragma once

#include <vector>

class FrameData;
class Device;
class World;
class CommandQueue;
class RenderGraph;
class Managers;

namespace enki
{
    class TaskScheduler;
};

class Frame
{
public:

    Frame();

    virtual ~Frame();

public:
    
    void setFrameCount(unsigned int frameCount);

    void setFrameIndex(unsigned int frameIndex);
    unsigned int getFrameIndex();

public:
    // setup frame data
    void createCommandList(std::shared_ptr<Device> device);
    void setDirectCommandQueue(std::shared_ptr<CommandQueue> directCommandQueue);
    void setWorld(std::shared_ptr<World> world);
    void setRenderGraph(RenderGraph* renderGraph);
    void setManagers(std::shared_ptr<Managers> managers);
    void setViewport(const D3D12_VIEWPORT& viewport);
    void setScissorRect(const D3D12_RECT& scissorRect); 
    void setTaskScheduler(std::shared_ptr<enki::TaskScheduler> taskScheduler);

    FrameData* getFrameData();

public:
    void beginFrame();
    void renderFrame();
    void endFrame();

protected:
    FrameData *mFrameData = nullptr;
    unsigned int mFrameCount = 0;
    unsigned int mFrameIndex = 0;
};