#pragma once
#include "BufferResource.h"
#include <d3d12.h>

class VertexBuffer : public BufferResource
{
public:

    VertexBuffer();

    ~VertexBuffer();

public:
    D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
};
