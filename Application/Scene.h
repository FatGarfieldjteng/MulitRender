#pragma once
#include <d3d12.h>
#include <vector>

class Mesh;
class GraphicsSystem;

class Scene
{
public:

    Scene(GraphicsSystem* GS);

    ~Scene();

public:

    virtual void init() = 0;
    
protected:
    GraphicsSystem* mGS;
};
