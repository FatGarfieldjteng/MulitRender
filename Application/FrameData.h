#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class FrameData
{
public:

    FrameData();

    virtual ~FrameData();

private:
    // use one commandList to record drawing commands
    ComPtr<ID3D12GraphicsCommandList> mCommandList;

};