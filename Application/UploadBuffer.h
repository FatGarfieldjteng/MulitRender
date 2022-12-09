#pragma once
#include "Mesh.h"
#include <d3d12.h>
#include <queue>
#include <memory>

class Device;

class UploadBuffer
{
public:

    struct Allocation
    {
        void* CPUAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);
    };

    // default page size is 2M
    UploadBuffer(std::shared_ptr<Device> device, size_t pageSize = 2 * 1024 * 1024);

    ~UploadBuffer();

public:
    Allocation allocate(size_t bufferSize, size_t alignment);

    void reset();

    size_t pageSize()
    {
        return mPageSize;
    }

private:
    struct Page
    {
        Page(std::shared_ptr<Device> device, size_t pageSize);
        ~Page();

        bool hasSpace(size_t sizeInBytes, size_t alignment) const;

        Allocation allocate(size_t sizeInBytes, size_t alignment);

        void reset();

    private:

        ComPtr<ID3D12Resource> mResource;

        // Base pointer.
        void* mCPUAddress = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);

        // Allocated page size.
        size_t mPageSize = 0;
        // Current allocation offset in bytes.
        size_t mOffset = 0;
    };

    std::shared_ptr<Page> acquirePage();

    using PagePool = std::deque< std::shared_ptr<Page> >;

    PagePool m_PagePool;
    PagePool m_AvailablePages;
    std::shared_ptr<Page> mCurrentPage;

    size_t mPageSize = 2048;

    std::shared_ptr<Device> mDevice;
};
