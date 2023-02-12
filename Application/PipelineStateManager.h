#pragma once

#include <map>
#include <string>
#include <memory>

class Device;
class PipelineState;
class RootSignatureManager;
class EffectManager;

class PipelineStateManager
{
public:

    PipelineStateManager(std::shared_ptr<Device> device, 
        std::shared_ptr <RootSignatureManager> rootSignatureManager,
        std::shared_ptr <EffectManager> effectManager);

    ~PipelineStateManager();

public:
    void createPipelineStates();

    void addPipelineState(const std::string& ID, ComPtr<ID3D12PipelineState> pipelineState);

    ComPtr<ID3D12PipelineState> getPipelineState(const std::string& ID);

private:
    void createSimplePipelineState();
    void createShadowPipelineState();

private:
    std::map<std::string, ComPtr<ID3D12PipelineState>> mStringToPipelineState;
    std::shared_ptr<Device> mDevice;
    std::shared_ptr <RootSignatureManager> mRootSignatureManager;
    std::shared_ptr <EffectManager> mEffectManager;
};
