#include "framework.h"
#include "Texture.h"
#include "Device.h"
#include "d3dx12.h"
#include "helper.h"

Texture::Texture(std::shared_ptr<Device>,
    ComPtr<ID3D12Resource> resource)
{
    createViews();
}

Texture::~Texture()
{

}

void Texture::createViews()
{
    if (mResource)
    {
        CD3DX12_RESOURCE_DESC desc(mResource->GetDesc());

        // RTV
        if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
        {
            mRTV = mDevice->allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            mDevice->createRenderTargetView(mResource.Get(), nullptr,
                mRTV.CPUDescriptorHandle());
        }
        // DSV
        if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
        {
            mDSV = mDevice->allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            mDevice->createDepthStencilView(mResource.Get(), nullptr,
                mDSV.CPUDescriptorHandle());
        }
        // SRV
        if (desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
        {
            mSRV = mDevice->allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            mDevice->createShaderResourceView(mResource.Get(), nullptr,
                mSRV.CPUDescriptorHandle());
        }
        // UAV for each mip (only supported for 1D and 2D textures).
        if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS &&
            desc.DepthOrArraySize == 1)
        {
            mUAV =
                mDevice->allocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, desc.MipLevels);
            for (int i = 0; i < desc.MipLevels; ++i)
            {
                auto uavDesc = getUAVDesc(desc, i);
                mDevice->createUnorderedAccessView(mResource.Get(), nullptr, &uavDesc,
                    mUAV.CPUDescriptorHandle(i));
            }
        }
    }
}

D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::getUAVDesc(const D3D12_RESOURCE_DESC& resDesc,
    UINT mipSlice, 
    UINT arraySlice,
    UINT planeSlice)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = resDesc.Format;

    switch (resDesc.Dimension)
    {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        if (resDesc.DepthOrArraySize > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            uavDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize - arraySlice;
            uavDesc.Texture1DArray.FirstArraySlice = arraySlice;
            uavDesc.Texture1DArray.MipSlice = mipSlice;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            uavDesc.Texture1D.MipSlice = mipSlice;
        }
        break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        if (resDesc.DepthOrArraySize > 1)
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize - arraySlice;
            uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
            uavDesc.Texture2DArray.PlaneSlice = planeSlice;
            uavDesc.Texture2DArray.MipSlice = mipSlice;
        }
        else
        {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.PlaneSlice = planeSlice;
            uavDesc.Texture2D.MipSlice = mipSlice;
        }
        break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uavDesc.Texture3D.WSize = resDesc.DepthOrArraySize - arraySlice;
        uavDesc.Texture3D.FirstWSlice = arraySlice;
        uavDesc.Texture3D.MipSlice = mipSlice;
        break;
    default:
        throw std::exception("Invalid resource dimension.");
    }

    return uavDesc;
}