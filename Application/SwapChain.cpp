#include "framework.h"
#include "SwapChain.h"
#include "helper.h"

std::shared_ptr<SwapChain> SwapChain::create(HWND hWnd,
    std::shared_ptr<Adapter> adapter,
    std::shared_ptr<Device> device,
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
    uint32_t width, uint32_t height,
    DXGI_FORMAT backBufferFormat)
{
    return std::make_shared<SwapChain>(hWnd,
        adapter,
        device,
        width, height,
        backBufferFormat);
}

SwapChain::SwapChain(HWND hWnd,
    std::shared_ptr<Adapter> adapter,
    std::shared_ptr<Device> device,
    uint32_t width, uint32_t height,
    DXGI_FORMAT backBufferFormat)
: mDevice(device)
, mHWnd( hWnd )
, mWidth(width)
, mHeight(height)
, mBackBufferFormat(backBufferFormat)
{
    auto d3d12CommandQueue = mDevice->directCommandQueue();

    // Query the factory from the adapter that was used to create the device.
    auto dxDevice     = mDevice->device();
    auto dxAdapter = adapter->adapter();

    // Get the factory that was used to create the adapter.
    ComPtr<IDXGIFactory>  dxgiFactory;
    ComPtr<IDXGIFactory5> dxgiFactory5;
    ThrowIfFailed(dxAdapter->GetParent( IID_PPV_ARGS( &dxgiFactory ) ) );
    // Now get the DXGIFactory5 so I can use the IDXGIFactory5::CheckFeatureSupport method.
    ThrowIfFailed( dxgiFactory.As( &dxgiFactory5 ) );

    // Check for tearing support.
    BOOL allowTearing = FALSE;
    if ( SUCCEEDED(
             dxgiFactory5->CheckFeatureSupport( DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof( BOOL ) ) ) )
    {
        mTearingSupported = ( allowTearing == TRUE );
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width                 = mWidth;
    swapChainDesc.Height                = mHeight;
    swapChainDesc.Format                = mBackBufferFormat;
    swapChainDesc.Stereo                = FALSE;
    swapChainDesc.SampleDesc            = { 1, 0 };
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount           = BufferCount;
    swapChainDesc.Scaling               = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = mTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    // Now create the swap chain.
    ComPtr<IDXGISwapChain1> dxgiSwapChain1;
    ThrowIfFailed( dxgiFactory5->CreateSwapChainForHwnd( d3d12CommandQueue.Get(), mHWnd, &swapChainDesc, nullptr,
                                                         nullptr, &dxgiSwapChain1 ) );

    // Cast to swapchain4
    ThrowIfFailed( dxgiSwapChain1.As(&mSwapChain));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    ThrowIfFailed( dxgiFactory5->MakeWindowAssociation(mHWnd, DXGI_MWA_NO_ALT_ENTER ) );

    // Initialize the current back buffer index.
    mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

    // Set maximum frame latency to reduce input latency.
    mSwapChain->SetMaximumFrameLatency( BufferCount - 1 );

    // Get the SwapChain's waitable object.
    mHFrameLatencyWaitableObject = mSwapChain->GetFrameLatencyWaitableObject();

    //UpdateRenderTargetViews();
}

SwapChain::~SwapChain()
{

}

void SwapChain::updateRenderTargetViews(ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    auto dxDevice = mDevice->device();

    auto rtvDescriptorSize = dxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < BufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        dxDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        mBackBuffers[i] = backBuffer;

        rtvHandle.Offset(rtvDescriptorSize);
    }
}

void SwapChain::present()
{
    UINT syncInterval = mVSync ? 1 : 0;
    UINT presentFlags = mTearingSupported && !mVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));
}
