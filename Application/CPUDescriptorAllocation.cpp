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
    uint32_t numHandles, uint32_t descriptorSize, 
    std::shared_ptr<CPUDescriptorPage> page)
    : mBaseDescriptor(baseDescriptor)
    , mNumHandles(numHandles)
    , mDescriptorSize(descriptorSize)
    , mPage(page)
{}

CPUDescriptorAllocation::~CPUDescriptorAllocation()
{
    free();
}


CPUDescriptorAllocation::CPUDescriptorAllocation(CPUDescriptorAllocation&& allocation)
    : mBaseDescriptor(allocation.mBaseDescriptor)
    , mNumHandles(allocation.mNumHandles)
    , mDescriptorSize(allocation.mDescriptorSize)
    , mPage(std::move(allocation.mPage))
{
    // clear allocation
    allocation.mBaseDescriptor.ptr = 0;
    allocation.mNumHandles = 0;
    allocation.mDescriptorSize = 0;
}


CPUDescriptorAllocation& CPUDescriptorAllocation::operator=(CPUDescriptorAllocation&& other)
{
    free();

    mBaseDescriptor = other.mBaseDescriptor;
    mNumHandles = other.mNumHandles;
    mDescriptorSize = other.mDescriptorSize;
    mPage = std::move(other.mPage);

    other.mBaseDescriptor.ptr = 0;
    other.mNumHandles = 0;
    other.mDescriptorSize = 0;

    return *this;
}

void CPUDescriptorAllocation::free()
{
    if (!isNull() && mPage)
    {
        mPage->Free(std::move(*this), Application::GetFrameCount());

        mBaseDescriptor.ptr = 0;
        mNumHandles = 0;
        mDescriptorSize = 0;
        mPage.reset();
    }
}