#include "framework.h"
#include "CPUDescriptorPage.h"
#include "Device.h"

CPUDescriptorPage::CPUDescriptorPage(std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
	:mHeapType(type),
	mNumDescriptorsPerHeap(numDescriptorsPerHeap)
{
    mDescriptorHeap = device->createDescriptorHeap(mHeapType, mNumDescriptorsPerHeap);

    mBaseDescriptor = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    mDescriptorHandleIncrementSize = device->getDescriptorHandleIncrementSize(mHeapType);
    mNumFreeDescriptors = mNumDescriptorsPerHeap;

    // Initialize the free lists
    addNewBlock(0, mNumFreeDescriptors);
}

CPUDescriptorPage::~CPUDescriptorPage()
{

}

void CPUDescriptorPage::addNewBlock(uint32_t offset, uint32_t numDescriptors)
{
    // map::emplace returns a pair <iterator, bool>
    auto offsetIt = mFreeListByOffset.emplace(offset, numDescriptors);

    // multimap returns a iterator
    auto sizeIt = mFreeListBySize.emplace(numDescriptors, offsetIt.first);

    // cross reference
    offsetIt.first->second.FreeListBySizeIt = sizeIt;
}

CPUDescriptorAllocation CPUDescriptorPage::allocate(uint32_t numDescriptors)
{
    std::lock_guard<std::mutex> lock(mMutex);

    // no space available, return null, early return
    if (numDescriptors > mNumFreeDescriptors)
    {
        return CPUDescriptorAllocation();
    }
    
    // no block meets the space, return null, early return
    // or, iterator to size multimap is acquired
    auto smallestBlockIt = mFreeListBySize.lower_bound(numDescriptors);
    if (smallestBlockIt == mFreeListBySize.end())
    {
        // There was no free block that could satisfy the request.
        return CPUDescriptorAllocation();
    }

    // a block that meets the allocate requirement found, continue
    // get block size
    uint32_t blockSize = smallestBlockIt->first;

    // offset multimap
    auto offsetIt = smallestBlockIt->second;

    // get block offset
    uint32_t offset = offsetIt->first;

    // remove entry from map and multimap, beacuse it should be allocated
    mFreeListBySize.erase(smallestBlockIt);
    mFreeListByOffset.erase(offsetIt);

    // split this block, return the first part, and add the second part into free map and multimap
    auto newOffset = offset + numDescriptors;
    auto newSize = blockSize - numDescriptors;

    if (newSize > 0)
    {
        // If the allocation didn't exactly match the requested size,
        // return the left-over to the free list.
        addNewBlock(newOffset, newSize);
    }

    // decrease free descriptors
    mNumFreeDescriptors -= numDescriptors;

    return CPUDescriptorAllocation(

        // the start of the descriptor that allocated
        CD3DX12_CPU_DESCRIPTOR_HANDLE(mBaseDescriptor, offset, mDescriptorHandleIncrementSize),

        // allocators
        numDescriptors, 
        
        // 
        mDescriptorHandleIncrementSize, 
        
        // shared_ptr of CPUDescriptorPage
        shared_from_this());
}

void CPUDescriptorPage::free(CPUDescriptorAllocation&& allocation)
{
    // get start of the allocation's base descriptor
    // computer its offset related to base desciptor of this page
    auto offset = computeOffset(allocation.CPUDescriptorHandle());

    std::lock_guard<std::mutex> lock(mMutex);

    // Don't add the block directly to the free list until the frame has completed.
    mStaleDescriptors.emplace(offset, allocation.numHandles());
}

void CPUDescriptorPage::freeBlock(uint32_t offset, uint32_t numDescriptors)
{
    // get the entry with key larger than offset
    auto nextBlockIt = mFreeListByOffset.upper_bound(offset);

    auto prevBlockIt = nextBlockIt;

    if (prevBlockIt != mFreeListByOffset.begin())
    {
        // if it's not the first block, prevBlockIt points to nextBlockIt's previous entry
        --prevBlockIt;
    }
    else
    {
        // only mark prevBlockIt as a special value
        prevBlockIt = mFreeListByOffset.end();
    }

    // Add the number of free handles back to the heap.
    // This needs to be done before merging any blocks since merging
    // blocks modifies the numDescriptors variable.
    mNumFreeDescriptors += numDescriptors;

    if (prevBlockIt != mFreeListByOffset.end() &&
        offset == prevBlockIt->first + prevBlockIt->second.Size)
    {
        // The previous block is exactly behind the block that is to be freed.
        //
        // PrevBlock.Offset           Offset
        // |                          |
        // |<-----PrevBlock.Size----->|<------Size-------->|
        //

        // Increase the block size by the size of merging with the previous block.
        offset = prevBlockIt->first;
        numDescriptors += prevBlockIt->second.Size;

        // Remove the previous block from the free list.
        mFreeListBySize.erase(prevBlockIt->second.FreeListBySizeIt);
        mFreeListByOffset.erase(prevBlockIt);
    }

    if (nextBlockIt != mFreeListByOffset.end() &&
        offset + numDescriptors == nextBlockIt->first)
    {
        // The next block is exactly in front of the block that is to be freed.
        //
        // Offset               NextBlock.Offset 
        // |                    |
        // |<------Size-------->|<-----NextBlock.Size----->|

        // Increase the block size by the size of merging with the next block.
        numDescriptors += nextBlockIt->second.Size;

        // Remove the next block from the free list.
        mFreeListBySize.erase(nextBlockIt->second.FreeListBySizeIt);
        mFreeListByOffset.erase(nextBlockIt);
    }

    // add the freed block to the free list.
    addNewBlock(offset, numDescriptors);
}

void CPUDescriptorPage::releaseStaleDescriptors()
{
    std::lock_guard<std::mutex> lock(mMutex);

    while (!mStaleDescriptors.empty())
    {
        auto& staleDescriptor = mStaleDescriptors.front();

        // The offset of the descriptor in the heap.
        auto offset = staleDescriptor.Offset;
        // The number of descriptors that were allocated.
        auto numDescriptors = staleDescriptor.Size;

        freeBlock(offset, numDescriptors);

        mStaleDescriptors.pop();
    }
}