#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

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

    struct InOutReource
    {
        std::shared_ptr < TextureResource > resource;
        std::string name;
        ResouceType type;
    };

    RenderPass();

    ~RenderPass();

public:
    void addInput(const std::string& name, 
        std::shared_ptr<TextureResource> resource, 
        ResouceType type);
    void addOutput(const std::string& name, 
        std::shared_ptr < TextureResource> resource, 
        ResouceType type);

    void setRootSignature(ComPtr<ID3D12RootSignature> rootSignature);
    void setPipelineState(ComPtr<ID3D12PipelineState> pipelineState);

public:
    void setName(const std::string& name);
    std::string getName() const;

    virtual void render(FrameData* frameData) = 0;

protected:

    std::string mName;

    std::vector <InOutReource> mInputResources;
    
    std::vector <InOutReource> mOutputResources;
    
    ComPtr<ID3D12RootSignature>     mRootSignature;

    ComPtr<ID3D12PipelineState>     mPipelineState;

    RenderTask* mRenerTask = nullptr;

    std::shared_ptr<enki::TaskScheduler> mTaskScheduler;
};
