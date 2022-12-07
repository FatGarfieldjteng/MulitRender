#include "framework.h"
#include "CPUDescriptorAllocation.h"
#include "CPUDescriptorPage.h"

CPUDescriptorAllocation::CPUDescriptorAllocation()
{

}

CPUDescriptorAllocation::~CPUDescriptorAllocation()
{

}

CPUDescriptorAllocation::CPUDescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor, 
    uint32_t numDescriptors, uint32_t descriptorHandleIncrementSize,
    std::shared_ptr<CPUDescriptorPage> page)
    : mBaseDescriptor(baseDescriptor)
    , mNumDescriptors(numDescriptors)
    , mDescriptorHandleIncrementSize(descriptorHandleIncrementSize)
    , mPage(page)
{}

CPUDescriptorAllocation::~CPUDescriptorAllocation()
{
    free();
}


CPUDescriptorAllocation::CPUDescriptorAllocation(CPUDescriptorAllocation&& allocation)
    : mBaseDescriptor(allocation.mBaseDescriptor)
    , mNumDescriptors(allocation.mNumDescriptors)
    , mDescriptorHandleIncrementSize(allocation.mDescriptorHandleIncrementSize)
    , mPage(std::move(allocation.mPage))
{
    // clear allocation
    allocation.mBaseDescriptor.ptr = 0;
    allocation.mNumDescriptors = 0;
    allocation.mDescriptorHandleIncrementSize = 0;
}


CPUDescriptorAllocation& CPUDescriptorAllocation::operator=(CPUDescriptorAllocation&& other)
{
    free();

    mBaseDescriptor = other.mBaseDescriptor;
    mNumDescriptors = other.mNumDescriptors;
    mDescriptorHandleIncrementSize = other.mDescriptorHandleIncrementSize;
    mPage = std::move(other.mPage);

    other.mBaseDescriptor.ptr = 0;
    other.mNumDescriptors = 0;
    other.mDescriptorHandleIncrementSize = 0;

    return *this;
}

void CPUDescriptorAllocation::free()
{
    if (!isNull() && mPage)
    {
        mPage->free(std::move(*this), Application::GetFrameCount());

        mBaseDescriptor.ptr = 0;
        mNumDescriptors = 0;
        mDescriptorHandleIncrementSize = 0;
        mPage.reset();
    }
}