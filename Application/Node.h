#pragma once

#include <memory>

class Mesh;

class Node
{
public:

    Node();

    ~Node();

public:
    void setMesh(std::shared_ptr<Mesh> mesh)
    {
        mMesh = mesh;
    }

    std::shared_ptr<Mesh> getMesh()
    {
        return mMesh;
    }

    void setWorldMatrix(DirectX::XMMATRIX worldMatrix)
    {
        mWorldMatrix = worldMatrix;
    }

    DirectX::XMMATRIX getWorldMatrix()
    {
        return mWorldMatrix;
    }

private:

    DirectX::XMMATRIX mWorldMatrix;
    std::shared_ptr<Mesh> mMesh;
};
