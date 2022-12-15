#pragma once
#include "GraphicsResource.h"
#include "CPUDescriptorAllocation.h"
#include <memory>
#include <d3d12.h>

class Device;

class Texture : public GraphicsResource
{
public:

    Texture(std::shared_ptr<Device> device, 
        ComPtr<ID3D12Resource> resource);

    ~Texture();

private:
    void createViews();

    D3D12_UNORDERED_ACCESS_VIEW_DESC getUAVDesc(const D3D12_RESOURCE_DESC& resDesc,
    UINT mipSlice,
    UINT arraySlice = 0,
    UINT planeSlice = 0);
private:
    std::shared_ptr<Device> mDevice;

public:
    CPUDescriptorAllocation mRTV;
    CPUDescriptorAllocation mDSV;
    CPUDescriptorAllocation mSRV;
    CPUDescriptorAllocation mUAV;
};
