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
};
