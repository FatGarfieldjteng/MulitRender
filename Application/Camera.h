#pragma once
#include <DirectXMath.h>
class Camera
{
public:
    Camera();

    ~Camera();

    DirectX::XMMATRIX modelMaxtrix()
    {
        return mModelMatrix;
    }

    void modelMaxtrix(const DirectX::XMMATRIX& value)
    {
        mModelMatrix = value;
    }

    DirectX::XMMATRIX viewMaxtrix()
    {
        return mViewMatrix;
    }

    void viewMaxtrix(const DirectX::XMMATRIX& value)
    {
        mViewMatrix = value;
    }

    void viewMaxtrix(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& up)
    {
        mViewMatrix = DirectX::XMMatrixLookAtLH(eye, target, up);
    }

    DirectX::XMMATRIX projectionMaxtrix()
    {
        return mProjectionMatrix;
    }

    void projectionMaxtrix(const DirectX::XMMATRIX& value)
    {
        mProjectionMatrix = value;
    }

    void projectionMaxtrix(float aspectRatio, float zNear, float zFar)
    {
        mProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(mFoV), aspectRatio, zNear, zFar);
    }

    void computeModelViewProjectionMatrix()
    {
        mModelViewProjectionMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(mModelMatrix, mViewMatrix), mProjectionMatrix);
    }

    DirectX::XMMATRIX modelViewProjectionMatrix()
    {
        return mModelViewProjectionMatrix;
    }

private:
    
    // intrinsic parameters

    // field of view
    float mFoV;

    // extrinsic parameters
    DirectX::XMVECTOR mEye;
    DirectX::XMVECTOR mTarget;
    DirectX::XMVECTOR mUp;

    DirectX::XMMATRIX mModelMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mViewMatrix = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX mProjectionMatrix = DirectX::XMMatrixIdentity();

    DirectX::XMMATRIX mModelViewProjectionMatrix = DirectX::XMMatrixIdentity();
};
