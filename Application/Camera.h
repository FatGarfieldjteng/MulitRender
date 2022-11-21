#pragma once
#include <DirectXMath.h>
class Camera
{
public:
    Camera();

    ~Camera();

private:
    float m_FoV;

    DirectX::XMMATRIX mModelMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mViewMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mProjectionMatrix = DirectX::XMMatrixIdentity();
};
