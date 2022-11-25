#pragma once
#include <d3d12.h>
#include <vector>

class Mesh;
class GraphicsSystem;

class Scene
{
public:

    Scene();

    ~Scene();

public:

    virtual void build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList) = 0;
    
};
