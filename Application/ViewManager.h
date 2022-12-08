#pragma once
#include <functional>
#include <memory>
#include <queue>
#include "d3dx12.h"

class CommandList;
class RootSignature;
class Device;

class ViewManager
{
public:

    ViewManager(std::shared_ptr<Device> device, 
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptorsPerHeap = 1024);

    ~ViewManager();

public:
    void stageCPUDescriptors(uint32_t rootParameterIndex, 
        uint32_t offset, uint32_t numCPUDescriptors, 
        const D3D12_CPU_DESCRIPTOR_HANDLE srcBaseCPUDescriptor);

    void commitStagedDescriptors(CommandList& commandList, 
        std::function<void(ID3D12GraphicsCommandList*, 
            UINT, 
            D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
    void commitStagedDescriptorsForDraw(CommandList& commandList);
    void commitStagedDescriptorsForDispatch(CommandList& commandList);

    D3D12_GPU_DESCRIPTOR_HANDLE copyDescriptor(CommandList& comandList, 
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

    void parseRootSignature(const RootSignature& rootSignature);

    void reset();

private:
    
    ComPtr<ID3D12DescriptorHeap> acquireDescriptorHeap();
        
    ComPtr<ID3D12DescriptorHeap> createDescriptorHeap();

    uint32_t computeStaleDescriptorCount() const;

private:

    // using uint32_t bit mask limits the number of descriptor tables to be 32
    static const uint32_t MaxDescriptorTables = 32;

    struct CPUDescriptorTableCacheEntry
    {
        void reset()
        {
            NumCPUDescriptors = 0;
            BaseCPUDescriptor = nullptr;
        }

        // The number of descriptors in this descriptor table.
        uint32_t NumCPUDescriptors = 0;
        // The pointer to the descriptor in the descriptor handle cache.
        D3D12_CPU_DESCRIPTOR_HANDLE* BaseCPUDescriptor = nullptr;
    };

    D3D12_DESCRIPTOR_HEAP_TYPE mDescriptorHeapType;

    uint32_t mNumDescriptorsPerHeap = 0;

    // The increment size of a descriptor.
    uint32_t mDescriptorHandleIncrementSize = 0;

    // cache for CPU descriptors
    std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> mCPUDescriptorHandleCache;

    // descriptor handle cache entry array
    // each entry corresponding to one desciptor table in root signature
    CPUDescriptorTableCacheEntry mCPUDescriptorTableCache[MaxDescriptorTables];

    uint32_t mDescriptorTableBitMask = 0;
    
    uint32_t mStaleDescriptorTableBitMask = 0;

    using DescriptorHeapPool = std::queue< ComPtr<ID3D12DescriptorHeap> >;

    DescriptorHeapPool mDescriptorHeapPool;
    DescriptorHeapPool mAvailableDescriptorHeaps;

    ComPtr<ID3D12DescriptorHeap> mCurrentDescriptorHeap;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrentGPUDescriptorHandle = D3D12_DEFAULT;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mCurrentCPUDescriptorHandle = D3D12_DEFAULT;

    uint32_t mNumFreeHandles = 0;
};
