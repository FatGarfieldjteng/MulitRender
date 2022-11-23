#pragma once
#include <d3d12.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <memory>

class Device;
class CommandQueue;
class GraphicsSystem;

class Mesh
{
public:

    Mesh(GraphicsSystem* GS);

    ~Mesh();

public:
    
    virtual void init() = 0;
        
protected:

    GraphicsSystem* mGS;
    VertexBuffer mVertexBuffer;
    IndexBuffer mIndexBuffer;
};
