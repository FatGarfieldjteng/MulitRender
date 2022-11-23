#pragma once
#include <d3d12.h>

class BufferResource
{
public:

    BufferResource();

    ~BufferResource();

public:
    virtual void init() = 0;

    ComPtr<ID3D12Resource> mBuffer;
};
