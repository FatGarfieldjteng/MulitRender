#pragma once
#include <d3d12.h>

class BufferResource
{
public:

    BufferResource();

    ~BufferResource();

public:
    ComPtr<ID3D12Resource> mBuffer;
};
