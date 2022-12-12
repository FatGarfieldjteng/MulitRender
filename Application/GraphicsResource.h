#pragma once
#include <wrl/client.h>
#include <d3d12.h>

class GraphicsResource
{
public:

    GraphicsResource();

    ~GraphicsResource();

public:
    ComPtr<ID3D12Resource> mResource;
};
