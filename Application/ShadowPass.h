#pragma once

#include "RenderPass.h"
#include <memory>

class Device;

class ShadowPass : public RenderPass
{
public:

    ShadowPass(std::shared_ptr<Device> device);

    ~ShadowPass();

public:
    virtual void buildTasks() override;

    virtual void preprocess() override;
    virtual void execute() override;
    virtual void postprocess() override;
};
