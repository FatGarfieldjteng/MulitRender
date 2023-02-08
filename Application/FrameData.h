#pragma once

#include <memory>
#include <vector>

class CommandList;
class Device;
class World;

class FrameData
{
public:
    FrameData();

    virtual ~FrameData();

public:
    void createCommandList(std::shared_ptr<Device> device);
    void setWorld(std::shared_ptr<World> world);
    void setViewport(const D3D12_VIEWPORT& viewport);
    void setScissorRect(const D3D12_RECT& scissorRect);
    void setBackBufferResource(ComPtr<ID3D12Resource> backBuffer);
    void setBackBufferView(const D3D12_CPU_DESCRIPTOR_HANDLE& backBufferView);
    void setDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilView);
    void setGraphicsRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
    void setPipelineState(ComPtr<ID3D12PipelineState> pipelineState);

public:
    void beginFrame();
    void renderFrame();
    void endFrame();
    void reset();

    std::vector<ID3D12GraphicsCommandList2*>& getCommandLists();

private:
    // This command list is single threaded
    // 

    // command list group
    std::unique_ptr<CommandList>    mclBeginFrame;
    std::unique_ptr<CommandList>    mclEndFrame;


    std::unique_ptr<CommandList>    mRenderCommandList;

    D3D12_VIEWPORT                  mViewport;
    D3D12_RECT                      mScissorRect;

    // back buffer
    ComPtr<ID3D12Resource>          mBackBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE     mBackBufferView;

    // depth stencil buffer
    D3D12_CPU_DESCRIPTOR_HANDLE     mDepthBufferView;

    ComPtr<ID3D12RootSignature>     mRootSignature;

    ComPtr<ID3D12PipelineState>     mPipelineState;

    std::shared_ptr<World>          mWorld;

    std::vector< ID3D12GraphicsCommandList2*> mDX12CommandLists;
    std::vector< CommandList*>       mCommandLists;

};