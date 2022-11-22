#pragma once
#include "Mesh.h"
#include <d3d12.h>

class CubeMesh : public Mesh
{
public:

    CubeMesh();

    ~CubeMesh();


public:

    virtual size_t vertexSize();
    virtual void init(std::shared_ptr<Device> device,
        std::shared_ptr<CommandQueue> commandQueue,
        ComPtr<ID3D12GraphicsCommandList2>);
};
