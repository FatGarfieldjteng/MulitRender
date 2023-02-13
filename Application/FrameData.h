#pragma once

#include <memory>
#include <vector>

class CommandList;
class Device;
class World;
class CommandQueue;
class RenderGraph;

class FrameData : public std::enable_shared_from_this<FrameData>
{
public:
    FrameData();

    virtual ~FrameData();

public:
    
    void createCommandList(std::shared_ptr<Device> device);
    void setWorld(std::shared_ptr<World> world);
    void setRenderGraph(std::shared_ptr<RenderGraph> renderGraph);
    void setDirectCommandQueue(std::shared_ptr<CommandQueue> directCommandQueue);
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
    void renderFrameA();
    uint64_t endFrame();
    void reset();

private:

    std::shared_ptr<CommandQueue> mDirectCommandQueue;

    // This command list is single threaded
    // 

    // command list group
    std::unique_ptr<CommandList>    mclBeginFrame;
    std::unique_ptr<CommandList>    mclEndFrame;

    std::unique_ptr<CommandList>    mclRender;

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

    std::shared_ptr<RenderGraph>    mRenderGraph;

    std::vector< CommandList*>       mCommandLists;

};