#pragma once
#include <DirectXMath.h>
class Light
{
public:
    Light();

    ~Light();

public:
    DirectX::XMFLOAT4 mPosition;
    DirectX::XMFLOAT4 mDirection;
    DirectX::XMFLOAT4 mColor;
    DirectX::XMFLOAT4 mFalloff;
};
