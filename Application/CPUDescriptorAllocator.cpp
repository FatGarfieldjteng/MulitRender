#include "framework.h"
#include "CPUDescriptorAllocator.h"
#include "CPUDescriptorPage.h"

CPUDescriptorAllocator::CPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorsPerHeap)
	:mHeapType(type)
	, mDescriptorsPerHeap(descriptorsPerHeap)
{

}

CPUDescriptorAllocator::~CPUDescriptorAllocator()
{

}

CPUDescriptorAllocation CPUDescriptorAllocator::allocate(uint32_t numDescriptors)
{
    std::lock_guard<std::mutex> lock(mMutex);

    CPUDescriptorAllocation allocation;

    for (auto iter = mAvailableHeaps.begin(); iter != mAvailableHeaps.end(); ++iter)
    {
        std::shared_ptr<CPUDescriptorPage> allocatorPage = mHeapPool[*iter];

        allocation = allocatorPage->allocate(numDescriptors);

        if (allocatorPage->numFreeDescriptors() == 0)
        {
            iter = mAvailableHeaps.erase(iter);
        }

        // A valid allocation has been found.
        if (!allocation.isNull())
        {
            break;
        }
    }
}

void CPUDescriptorAllocator::releaseStaleDescriptors(uint64_t frameNumber)
{
    std::lock_guard<std::mutex> lock(mMutex);

    for (size_t i = 0; i < mHeapPool.size(); ++i)
    {
        auto page = mHeapPool[i];

        page->releaseStaleDescriptors(frameNumber);

        if (page->numFreeDescriptors() > 0)
        {
            mAvailableHeaps.insert(i);
        }
    }
}

std::shared_ptr<CPUDescriptorPage> CPUDescriptorAllocator::CPUDescriptorAllocator::createPage()
{
	std::shared_ptr<CPUDescriptorPage> newPage = std::make_shared<CPUDescriptorPage>(mHeapType, mDescriptorsPerHeap);

	mHeapPool.emplace_back(newPage);
	mAvailableHeaps.insert(mHeapPool.size() - 1);

	return newPage;
}