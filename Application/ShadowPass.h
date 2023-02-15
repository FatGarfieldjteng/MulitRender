#pragma once

#include "RenderPass.h"
#include <memory>

class Device;

class ShadowPass : public RenderPass
{
public:

    ShadowPass();

    ~ShadowPass();

public:
    virtual void render(FrameData* frameData) override;
};
