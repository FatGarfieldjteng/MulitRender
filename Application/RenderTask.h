#pragma once

#include <TaskScheduler.h>

// each RenderTask is in charge of drawing a set of meshes

class FrameData;

class RenderTask : public enki::ITaskSet
{
public:
    RenderTask(uint32_t setSize, uint32_t minRange);

    ~RenderTask();

public:
    void ExecuteRange(enki::TaskSetPartition range, uint32_t threadnum) override {

        for (uint32_t index = range.start; index < range.end; ++index)
        {
            //do something
        }

    }
};
