#pragma once
#include "Effect.h"
#include <d3d12.h>

class ShadowEffect : public Effect
{
public:

    ShadowEffect();

    ~ShadowEffect();

protected:
    virtual void loadShader();
    virtual void createRootSignature(std::shared_ptr<Device> device);
    virtual void createPipelineStateObject(std::shared_ptr<Device> device);
};
