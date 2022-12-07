#pragma once
#include "CPUDescriptorAllocation.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <set>
#include <mutex>

class CPUDescriptorPage;
class Device;

class CPUDescriptorAllocator
{
public:

    CPUDescriptorAllocator(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorsPerHeap = 256);

    ~CPUDescriptorAllocator();

public:

    CPUDescriptorAllocation allocate(uint32_t numDescriptors = 1);

    void releaseStaleDescriptors();

private:
    using DescriptorHeapPool = std::vector< std::shared_ptr<CPUDescriptorPage> >;

    // Create a new heap with a specific number of descriptors.
    std::shared_ptr<CPUDescriptorPage> createPage();

    std::shared_ptr<Device> mDevice;

    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
    uint32_t mDescriptorsPerHeap = 0;

    DescriptorHeapPool mHeapPool;
    // Indices of available heaps in the heap pool.
    std::set<size_t> mAvailableHeaps;

    std::mutex mMutex;
};
