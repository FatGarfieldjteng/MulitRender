#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>
#include <vector>
#include <string>

class Adapter;

class Adapter
{
public:

    static std::shared_ptr<Adapter> Create( DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                            bool                useWarp       = false );

    Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter() const
    {
        return mAdapter;
    }

    /**
     * Get the description of the adapter.
     */
    const std::wstring GetDescription() const;

    Adapter(Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter);
    virtual ~Adapter() = default;
    

private:
    Microsoft::WRL::ComPtr<IDXGIAdapter4> mAdapter;
    DXGI_ADAPTER_DESC3                    mDesc;
};
