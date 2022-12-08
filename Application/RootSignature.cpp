#include "framework.h"
#include "RootSignature.h"
#include "Device.h"
#include "d3dx12.h"
#include "helper.h"

RootSignature::RootSignature()
{}

RootSignature::RootSignature(std::shared_ptr<Device> device,
    const D3D12_ROOT_SIGNATURE_DESC1& desc, 
    D3D_ROOT_SIGNATURE_VERSION version)
    :mDevice(device)
{
    setDesc(desc, version);
}

RootSignature::~RootSignature()
{
    destroy();
}

void RootSignature::destroy()
{
    for (UINT i = 0; i < mDesc.NumParameters; ++i)
    {
        const D3D12_ROOT_PARAMETER1& rootParameter = mDesc.pParameters[i];
        if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            delete[] rootParameter.DescriptorTable.pDescriptorRanges;
        }
    }

    delete[] mDesc.pParameters;
    mDesc.pParameters = nullptr;
    mDesc.NumParameters = 0;

    delete[] mDesc.pStaticSamplers;
    mDesc.pStaticSamplers = nullptr;
    mDesc.NumStaticSamplers = 0;

    mDescriptorTableBitMask = 0;
    mSamplerTableBitMask = 0;

    memset(mNumDescriptorsPerTable, 0, sizeof(mNumDescriptorsPerTable));
}

void RootSignature::setDesc(const D3D12_ROOT_SIGNATURE_DESC1& desc,
    D3D_ROOT_SIGNATURE_VERSION version)
{
    // release previous data
    destroy();

    UINT numParameters = desc.NumParameters;
    D3D12_ROOT_PARAMETER1* pParameters = numParameters > 0 ? new D3D12_ROOT_PARAMETER1[numParameters] : nullptr;

    for (UINT i = 0; i < numParameters; ++i)
    {
        const D3D12_ROOT_PARAMETER1& rootParameter = desc.pParameters[i];
        pParameters[i] = rootParameter;

        if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            UINT numDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;
            D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = numDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges] : nullptr;

            memcpy(pDescriptorRanges, rootParameter.DescriptorTable.pDescriptorRanges,
                sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

            pParameters[i].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
            pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

            // Set the bit mask depending on the type of descriptor table.
            if (numDescriptorRanges > 0)
            {
                switch (pDescriptorRanges[0].RangeType)
                {
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                    mDescriptorTableBitMask |= (1 << i);
                    break;
                case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                    mSamplerTableBitMask |= (1 << i);
                    break;
                }
            }

            // Count the number of descriptors in the descriptor table.
            for (UINT j = 0; j < numDescriptorRanges; ++j)
            {
                mNumDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
            }
        }
    }

    mDesc.NumParameters = numParameters;
    mDesc.pParameters = pParameters;

    UINT numStaticSamplers = desc.NumStaticSamplers;
    D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

    if (pStaticSamplers)
    {
        memcpy(pStaticSamplers, desc.pStaticSamplers,
            sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
    }

    mDesc.NumStaticSamplers = numStaticSamplers;
    mDesc.pStaticSamplers = pStaticSamplers;

    D3D12_ROOT_SIGNATURE_FLAGS flags = desc.Flags;
    mDesc.Flags = flags;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
    versionRootSignatureDesc.Init_1_1(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);

    // Serialize the root signature.
    Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc,
        version, &rootSignatureBlob, &errorBlob));

    // Create the root signature.
    mDevice->CreateRootSignature(rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

uint32_t RootSignature::descriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const
{
    uint32_t descriptorTableBitMask = 0;
    switch (descriptorHeapType)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        descriptorTableBitMask = mSamplerTableBitMask;
        break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        descriptorTableBitMask = mDescriptorTableBitMask;
        break;
    }

    return descriptorTableBitMask;
}

uint32_t RootSignature::numDescriptors(uint32_t rootIndex) const
{
    assert(rootIndex < 32);
    return mNumDescriptorsPerTable[rootIndex];
}