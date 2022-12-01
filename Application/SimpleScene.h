#pragma once
#include "Scene.h"
#include <d3d12.h>
#include <vector>

class Mesh;
class GraphicsSystem;

class SimpleScene : public Scene
{
public:

    SimpleScene();

    ~SimpleScene();

public:

    virtual void build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList);

    virtual void endBuild();

    virtual size_t mesheCount();

    virtual Mesh* mesh(size_t i);
    
private:
    std::vector <Mesh*> mMeshes;
};
