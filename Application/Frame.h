#pragma once

class FrameData;
class Device;
class World;

#include <vector>

class Frame
{
public:

    Frame();

    virtual ~Frame();

public:
    
    void frameCount(unsigned int value);

    void frameIndex(unsigned int value);
    unsigned int frameIndex();

public:
    // setup frame data
    void createCommandList(std::shared_ptr<Device> device);
    void setWorld(std::shared_ptr<World> world);
    void setViewport(const D3D12_VIEWPORT& viewport);
    void setScissorRect(const D3D12_RECT& scissorRect);
    void setBackBufferResource(ComPtr<ID3D12Resource> backBuffer);
    void setBackBufferView(const D3D12_CPU_DESCRIPTOR_HANDLE& backBufferView);
    void setDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilView);
    void setGraphicsRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
    void setPipelineState(ComPtr<ID3D12PipelineState> pipelineState);
    

    void beginFrame();
    void renderFrame();
    void endFrame();
    void reset();
    
    std::unique_ptr<CommandList>& getCommandList();

    std::vector<ID3D12CommandList*>& getCommandLists();

protected:
    FrameData *mFrameData = nullptr;
    unsigned int mFrameCount = 0;
    unsigned int mFrameIndex = 0;
};