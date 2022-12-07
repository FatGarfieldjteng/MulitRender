#pragma once

#include <memory>

class CPUDescriptorPage;

class CPUDescriptorAllocation
{
public:

    CPUDescriptorAllocation();

    CPUDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor,
        uint32_t numDescriptors, uint32_t descriptorHandleIncrementSize,
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
        assert(offset < mNumDescriptors);
        return { mBaseDescriptor.ptr + (mDescriptorHandleIncrementSize * offset) };
    }

    uint32_t numHandles() const
    {
        return mNumDescriptors;
    }

    std::shared_ptr<CPUDescriptorPage> page() const
    {
        return mPage;
    }

private:
    // Free the descriptor back to the heap it came from.
    void free();

    D3D12_CPU_DESCRIPTOR_HANDLE mBaseDescriptor;

    uint32_t mNumDescriptors = 0;

    uint32_t mDescriptorHandleIncrementSize = 0;

    // A pointer back to the original page where this allocation came from.
    std::shared_ptr<CPUDescriptorPage> mPage;
};
