#pragma once
#include <d3d12.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <memory>

class Device;
class CommandQueue;

class Mesh
{
public:

    Mesh();

    ~Mesh();

public:
    virtual size_t vertexSize() = 0;

    virtual void init(std::shared_ptr<Device> device,
        std::shared_ptr<CommandQueue> commandQueue,
        ComPtr<ID3D12GraphicsCommandList2>) = 0;
    
private:
    VertexBuffer mVertexBuffer;
    IndexBuffer mIndexBuffer;
};
