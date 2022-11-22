#pragma once
#include <d3d12.h>

class Device;

class Mesh
{
public:

    Mesh();

    ~Mesh();

public:
    virtual void init() = 0;
    
private:
    // vertex buffer
    ComPtr<ID3D12Resource> mVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

    // index buffer
    ComPtr<ID3D12Resource> mIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};
