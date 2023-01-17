#pragma once

#include "RenderPass.h"
#include <memory>

class Device;

class BeautyPass : public RenderPass
{
public:

    BeautyPass(std::shared_ptr<Device> device);

    ~BeautyPass();

public:
    virtual void buildTasks() override;

    // setup render target, root signature, viewport, scissor rect
    virtual void preprocess() override;
    // run tasks
    virtual void execute() override;
    // clear
    virtual void postprocess() override;
};
