#pragma once

#include <memory>
#include <vector>
#include <string>

class TextureResource;
class CommandList;
class RenderTask;
class Device;
class FrameData;

namespace enki {
    class TaskScheduler;
}

class RenderPass
{
public:

    enum ResouceType {
        ResourceType_BackBuffer,
        ResourceType_DepthStencil,
        ResourceType_RenderTarget,
        ResourceType_ShaderResource,
    };

    RenderPass();

    ~RenderPass();

public:
    void addInput(std::shared_ptr<TextureResource> resource, ResouceType resourceType);
    void addOutput(std::shared_ptr < TextureResource> resource, ResouceType resourceType);

    void setRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
    void setPipelineState(ComPtr<ID3D12PipelineState> pipelineState);

public:
    void setName(const std::string& name);
    std::string getName() const;

    virtual void render(FrameData* frameData) = 0;

protected:

    std::string mName;
    std::vector < std::shared_ptr<TextureResource> > mInputResources;
    std::vector <ResouceType> mInputResourceType;
    std::vector < std::shared_ptr<TextureResource> > mOutputResources;
    std::vector <ResouceType> mOutputResourceType;

    ComPtr<ID3D12RootSignature>     mRootSignature;

    ComPtr<ID3D12PipelineState>     mPipelineState;

    RenderTask* mRenerTask = nullptr;

    std::shared_ptr<enki::TaskScheduler> mTaskScheduler;
};
