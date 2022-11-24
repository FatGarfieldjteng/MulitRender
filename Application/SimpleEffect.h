#pragma once
#include "Effect.h"
#include <d3d12.h>

class SimpleEffect : public Effect
{
public:

    SimpleEffect(std::shared_ptr<Device> device);

    ~SimpleEffect();

protected:
    virtual void loadShader();
    virtual void createInputLayout();
};
