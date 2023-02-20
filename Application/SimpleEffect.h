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
};
