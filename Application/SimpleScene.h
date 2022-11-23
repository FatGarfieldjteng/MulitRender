#pragma once
#include "Scene.h"
#include <d3d12.h>
#include <vector>

class Mesh;
class GraphicsSystem;

class SimpleScene : public Scene
{
public:

    SimpleScene(GraphicsSystem* GS);

    ~SimpleScene();

public:

    virtual void init();
    
private:
    std::vector <Mesh*> mMeshes;
};
