#include "framework.h"
#include "Device.h"
#include "Adapter.h"
#include "helper.h"

std::shared_ptr<Device> Device::create( std::shared_ptr<Adapter> adapter )
{
    return std::make_shared<Device>(adapter);
}

Device::Device( std::shared_ptr<Adapter> adapter )
{
    auto dxgiAdapter = adapter->adapter();

    ThrowIfFailed( D3D12CreateDevice( dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &mDevice) ) );

    // Enable debug messages (only works if the debug layer has already been enabled).
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if ( SUCCEEDED(mDevice.As( &pInfoQueue ) ) )
    {

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,  // I'm really not sure how to avoid this
                                                                           // message.

            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                     // debugging.

            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                       // debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        // NewFilter.DenyList.NumCategories = _countof(Categories);
        // NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof( Severities );
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs        = _countof( DenyIds );
        NewFilter.DenyList.pIDList       = DenyIds;

        ThrowIfFailed( pInfoQueue->PushStorageFilter( &NewFilter ) );
    }

}

Device::~Device() 
{
}

void Device::createDirectCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mDirectCommandQueue)));
}

ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, 
    uint32_t numDescriptors)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;

    ThrowIfFailed(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

ComPtr<ID3D12CommandAllocator> Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(mDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> Device::CreateCommandList(ComPtr<ID3D12CommandAllocator> commandAllocator, 
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(mDevice->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    ThrowIfFailed(commandList->Close());

    return commandList;
}

ComPtr<ID3D12Fence> Device::CreateFence()
{
    ComPtr<ID3D12Fence> fence;

    ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}