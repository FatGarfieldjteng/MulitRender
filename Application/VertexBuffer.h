#pragma once
#include "BufferResource.h"
#include <d3d12.h>

class VertexBuffer : public BufferResource
{
public:

    VertexBuffer();

    ~VertexBuffer();

public:
    virtual void init();
};
