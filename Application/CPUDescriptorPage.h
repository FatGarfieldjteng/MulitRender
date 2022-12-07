#pragma once

#include "CPUDescriptorAllocation.h"
#include <wrl/client.h>
#include <map>
#include <queue>
#include <mutex>
#include "d3dx12.h"

class Device;

class CPUDescriptorPage : public std::enable_shared_from_this<CPUDescriptorPage>
{
public:

    CPUDescriptorPage(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap);

    ~CPUDescriptorPage();

private:
    
    struct StaleDescriptorInfo
    {
        StaleDescriptorInfo(uint32_t offset, uint32_t size, uint64_t frame)
            : Offset(offset)
            , Size(size)
            , FrameNumber(frame)
        {}

        // The offset within the descriptor heap.
        uint32_t Offset;
        // The number of descriptors
        uint32_t Size;
        // The frame number that the descriptor was freed.
        uint64_t FrameNumber;
    };

public:
    D3D12_DESCRIPTOR_HEAP_TYPE heapType() const
    {
        return mHeapType;
    }

    bool hasSpace(uint32_t numDescriptors) const
    {
        // reuturn true if list element has free descriptors that is equal or larger than numDescriptors
        return mFreeListBySize.lower_bound(numDescriptors) != mFreeListBySize.end();
    }

    uint32_t numFreeDescriptors() const
    {
        return mNumFreeDescriptors;
    }
    
    CPUDescriptorAllocation allocate(uint32_t numDescriptors);

    void free(CPUDescriptorAllocation&& allocation, uint64_t frameNumber);
    
    void releaseStaleDescriptors(uint64_t frameNumber);

protected:
    
    uint32_t computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        return static_cast<uint32_t>(descriptor.ptr - mBaseDescriptor.ptr) / mDescriptorHandleIncrementSize;
    }
    
    void addNewBlock(uint32_t offset, uint32_t numDescriptors);
    
    void freeBlock(uint32_t offset, uint32_t numDescriptors);

private:
    struct FreeBlockInfo;

    // the key is offset
    using FreeListByOffset = std::map<uint32_t, FreeBlockInfo>;

    // the key is size
    using FreeListBySize = std::multimap<uint32_t, FreeListByOffset::iterator>;

    struct FreeBlockInfo
    {
        FreeBlockInfo(uint32_t size)
            : Size(size)
        {}

        uint32_t Size;
        FreeListBySize::iterator FreeListBySizeIt;
    };

    using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

    FreeListByOffset mFreeListByOffset;
    FreeListBySize mFreeListBySize;

    // stage for descriptors need to be freed
    StaleDescriptorQueue mStaleDescriptors;

    ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE mHeapType;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mBaseDescriptor;
    uint32_t mDescriptorHandleIncrementSize = 0;

    // total number of descriptors
    uint32_t mNumDescriptorsPerHeap = 0;

    // number of free descriptors
    uint32_t mNumFreeDescriptors = 0;

    std::mutex mMutex;
};
