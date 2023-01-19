#pragma once
#include "Mesh.h"
#include <d3d12.h>

class CubeMesh : public Mesh
{
public:

    CubeMesh();

    ~CubeMesh();


public:

    virtual void build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList) override;

};
