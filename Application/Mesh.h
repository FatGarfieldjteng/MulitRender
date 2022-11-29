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

    Mesh();

    ~Mesh();

public:
    
    virtual void build(GraphicsSystem* GS, ComPtr<ID3D12GraphicsCommandList2> commandList) = 0;
        
protected:
    VertexBuffer mVertexBuffer;
    IndexBuffer mIndexBuffer;
    ComPtr<ID3D12Resource> mIntermediateVertexBuffer;
    ComPtr<ID3D12Resource> mIntermediateIndexBuffer;
};
