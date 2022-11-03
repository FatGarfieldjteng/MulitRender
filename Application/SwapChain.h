#pragma once

#include <dxgi1_6.h>     // For IDXGISwapChain4
#include <wrl/client.h>  // For Microsoft::WRL::ComPtr
#include <d3d12.h>

#include "Adapter.h"
#include "Device.h"
#include "d3dx12.h"
#include <memory>  // For std::shared_ptr

class SwapChain
{
public:

    // number of backbuffers
    static const UINT BufferCount = 3;

    static std::shared_ptr<SwapChain> create(HWND hWnd,
        std::shared_ptr<Adapter> adapter,
        std::shared_ptr<Device> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
        uint32_t width, uint32_t height, 
        DXGI_FORMAT backBufferFormat);

    bool IsTearingSupported() const
    {
        return mTearingSupported;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain4> getDXGISwapChain() const
    {
        return mSwapChain;
    }

    void updateRenderTargetViews(ComPtr<ID3D12DescriptorHeap> descriptorHeap);

    ComPtr<ID3D12Resource> backBuffer(UINT index)
    {
        return mBackBuffers[index];
    }

    UINT getCurrentBackBufferIndex()
    {
        return mSwapChain->GetCurrentBackBufferIndex();
    }

    void present();

public:
    // Swap chains can only be created through the Device.
    SwapChain(HWND hWnd,
        std::shared_ptr<Adapter> adapter,
        std::shared_ptr<Device> device,
        uint32_t width, uint32_t height, 
        DXGI_FORMAT mBackBufferFormat);
    virtual ~SwapChain();

private:
    bool mTearingSupported = false;
    std::shared_ptr<Device> mDevice;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain;
    HWND mHWnd = 0;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    DXGI_FORMAT mBackBufferFormat;
    UINT   mCurrentBackBufferIndex = 0;
    HANDLE mHFrameLatencyWaitableObject = 0;

    ComPtr<ID3D12Resource> mBackBuffers[BufferCount];
    
    bool mVSync = false;
    
};
