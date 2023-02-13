#pragma once

#include "RenderPass.h"
#include <memory>

class Device;

class BeautyPass : public RenderPass
{
public:

    BeautyPass();

    ~BeautyPass();

public:
    
    // run tasks
    virtual void render(std::shared_ptr<FrameData> frameData) override;

};
