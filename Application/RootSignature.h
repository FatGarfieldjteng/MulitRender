#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>

class Device;

class RootSignature
{
public:

    RootSignature();

    RootSignature(std::shared_ptr<Device> device, 
        const D3D12_ROOT_SIGNATURE_DESC1& desc,
        D3D_ROOT_SIGNATURE_VERSION version);

    ~RootSignature();

    void destroy();

    ComPtr<ID3D12RootSignature> rootSignature() const
    {
        return mRootSignature;
    }

    void setDesc(const D3D12_ROOT_SIGNATURE_DESC1& desc,
        D3D_ROOT_SIGNATURE_VERSION version);

    const D3D12_ROOT_SIGNATURE_DESC1& desc() const
    {
        return mDesc;
    }

    uint32_t descriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
    uint32_t numDescriptors(uint32_t rootIndex) const;

private:
    std::shared_ptr<Device> mDevice;

    D3D12_ROOT_SIGNATURE_DESC1 mDesc = {};
    ComPtr<ID3D12RootSignature> mRootSignature;

    // maximun number of descriptor tables is 32 because uint32_t is used aas mask
    // each entry of mNumDescriptorsPerTable stores number of descriptors in ith root parameter
    // if ith root parameter is a table parameter
    uint32_t mNumDescriptorsPerTable[32] = { 0 };

    // bit i == 1 means the ith rootParameter is a sampler table
    uint32_t mSamplerTableBitMask = 0;

    // bit i == 1 means the ith rootParameter is a CBV_UAV_SRV table
    uint32_t mDescriptorTableBitMask = 0;
};
