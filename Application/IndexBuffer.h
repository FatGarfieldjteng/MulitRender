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
};
