#pragma once
#include "GraphicsResource.h"
#include <d3d12.h>

class TextureResource : public GraphicsResource
{
public:

    TextureResource();
 
    ~TextureResource();

public:
    D3D12_CPU_DESCRIPTOR_HANDLE mRTV;
    D3D12_CPU_DESCRIPTOR_HANDLE mDSV;

    // GPU handle
    D3D12_GPU_DESCRIPTOR_HANDLE mSRV;
};
