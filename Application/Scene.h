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

    virtual void beginBuild()
    {

    }

    virtual void build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList) = 0;

    virtual void endBuild()
    {

    }

    virtual size_t mesheCount() = 0;

    virtual Mesh* mesh(size_t i) = 0;
};
