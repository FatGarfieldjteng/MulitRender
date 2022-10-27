#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

class Adapter;

class Device
{
public:

    static std::shared_ptr<Device> create( std::shared_ptr<Adapter> adapter = nullptr );

    Microsoft::WRL::ComPtr<ID3D12Device2> device() const
    {
        return mDevice;
    }

    explicit Device(std::shared_ptr<Adapter> adapter);

    virtual ~Device();

public:
    // create resources
    
    // direct command queue
    void createDirectCommandQueue();

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> directCommandQueue()
    {
        return mDirectCommandQueue;
    }

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, 
        uint32_t numDescriptors);

private:
    Microsoft::WRL::ComPtr<ID3D12Device2> mDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mDirectCommandQueue;

};