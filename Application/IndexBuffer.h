#pragma once
#include "BufferResource.h"
#include <d3d12.h>

class IndexBuffer : public BufferResource
{
public:

    IndexBuffer();

    ~IndexBuffer();

public:
    virtual void init();

    D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};
