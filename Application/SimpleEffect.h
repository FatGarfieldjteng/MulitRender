#pragma once
#include "Effect.h"
#include <d3d12.h>

class SimpleEffect : public Effect
{
public:

    SimpleEffect();

    ~SimpleEffect();

protected:
    virtual void loadShader();
    virtual void createRootSignature(std::shared_ptr<Device> device);
    virtual void createPipelineStateObject(std::shared_ptr<Device> device);
};
