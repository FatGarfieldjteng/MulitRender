#pragma once
#include "Mesh.h"
#include <d3d12.h>

class BufferResource
{
public:

    BufferResource();

    ~BufferResource();

public:
    virtual void init() = 0;
};
