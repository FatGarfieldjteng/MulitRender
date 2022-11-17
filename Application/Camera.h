#pragma once
#include <DirectXMath.h>
class Camera
{
public:
    Camera();

    ~Camera();

private:
    float m_FoV;

    DirectX::XMMATRIX m_ModelMatrix;
    DirectX::XMMATRIX m_ViewMatrix;
    DirectX::XMMATRIX m_ProjectionMatrix;
};
