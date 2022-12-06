#pragma once

#include <memory>

class CPUDescriptorPage;

class CPUDescriptorAllocation
{
public:

    CPUDescriptorAllocation();

    CPUDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
        uint32_t numHandles, uint32_t descriptorSize, 
        std::shared_ptr<CPUDescriptorPage> page);

    ~CPUDescriptorAllocation();

    // copies are not allowed
    CPUDescriptorAllocation(const CPUDescriptorAllocation&) = delete;
    CPUDescriptorAllocation& operator=(const CPUDescriptorAllocation&) = delete;

    // move is allowed
    CPUDescriptorAllocation(CPUDescriptorAllocation&& allocation);
    CPUDescriptorAllocation& operator=(CPUDescriptorAllocation&& other);

public:
    bool isNull() const
    {
        return mBaseDescriptor.ptr == 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle(uint32_t offset = 0) const
    {
        assert(offset < mNumHandles);
        return { mBaseDescriptor.ptr + (mDescriptorSize * offset) };
    }

    uint32_t GetNumHandles() const
    {
        return mNumHandles;
    }

    std::shared_ptr<CPUDescriptorPage> GetCPUDescriptorPage() const
    {
        return mPage;
    }


private:
    // Free the descriptor back to the heap it came from.
    void free();

    D3D12_CPU_DESCRIPTOR_HANDLE mBaseDescriptor;

    uint32_t mNumHandles = 0;

    uint32_t mDescriptorSize = 0;

    // A pointer back to the original page where this allocation came from.
    std::shared_ptr<CPUDescriptorPage> mPage;
};
