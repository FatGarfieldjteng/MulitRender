#include "framework.h"
#include "UploadBuffer.h"
#include "Device.h"
#include "d3dx12.h"
#include "helper.h"

UploadBuffer::UploadBuffer(std::shared_ptr<Device> device, size_t pageSize)
	:mPageSize(pageSize),
    mDevice(device)
{

}

UploadBuffer::~UploadBuffer()
{

}

UploadBuffer::Allocation UploadBuffer::allocate(size_t bufferSize, size_t alignment)
{
    if (bufferSize > mPageSize)
    {
        throw std::bad_alloc();
    }

    // If there is no current page, or the requested allocation exceeds the
    // remaining space in the current page, request a new page.
    if (!mCurrentPage || !mCurrentPage->hasSpace(bufferSize, alignment))
    {
        mCurrentPage = acquirePage();
    }

    return mCurrentPage->allocate(bufferSize, alignment);
}

std::shared_ptr<UploadBuffer::Page> UploadBuffer::acquirePage()
{
    std::shared_ptr<Page> page;

    if (!m_AvailablePages.empty())
    {
        page = m_AvailablePages.front();
        m_AvailablePages.pop_front();
    }
    else
    {
        page = std::make_shared<Page>(mDevice, mPageSize);
        m_PagePool.push_back(page);
    }

    return page;
}

void UploadBuffer::reset()
{
    mCurrentPage = nullptr;

    // reset all available pages.
    m_AvailablePages = m_PagePool;

    for (auto page : m_AvailablePages)
    {
        // reset the page for new allocations.
        page->reset();
    }
}

UploadBuffer::Page::Page(std::shared_ptr<Device> device, size_t pageSize)
    : mPageSize(pageSize)
{
    device->createCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(mPageSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&mResource));

    mGPUAddress = mResource->GetGPUVirtualAddress();
    mResource->Map(0, nullptr, &mCPUAddress);
}

UploadBuffer::Page::~Page()
{
    mResource->Unmap(0, nullptr);
    mCPUAddress = nullptr;
    mGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool UploadBuffer::Page::hasSpace(size_t bufferSize, size_t alignment) const
{
    size_t alignedSize = Math::AlignUp(bufferSize, alignment);
    size_t alignedOffset = Math::AlignUp(mOffset, alignment);

    return alignedOffset + alignedSize <= mPageSize;
}

UploadBuffer::Allocation UploadBuffer::Page::allocate(size_t bufferSize, size_t alignment)
{
    if (!hasSpace(bufferSize, alignment))
    {
        // Can't allocate space from page.
        throw std::bad_alloc();
    }

    size_t alignedSize = Math::AlignUp(bufferSize, alignment);
    mOffset = Math::AlignUp(mOffset, alignment);

    Allocation allocation;
    allocation.CPUAddress = static_cast<uint8_t*>(mCPUAddress) + mOffset;
    allocation.GPUAddress = mGPUAddress + mOffset;

    mOffset += alignedSize;

    return allocation;
}

void UploadBuffer::Page::reset()
{
    mOffset = 0;
}