#pragma once

#include <memory>

class Device;
class ShaderManager;
class EffectManager;
class RootSignatureManager;
class PipelineStateManager;
class TextureManager;

class Managers
{
public:

    Managers(std::shared_ptr<Device> device);

    ~Managers();

public:
 
    void createManagers();

    std::shared_ptr<ShaderManager> getShaderManager()
    {
        return mShaderManager;
    }

    std::shared_ptr<EffectManager> getEffectManager()
    {
        return mEffectManager;
    }

    std::shared_ptr<RootSignatureManager> getRootSignatureManager()
    {
        return mRootSignatureManager;
    }

    std::shared_ptr<PipelineStateManager> getPipelineStateManager()
    {
        return mPipelineStateManager;
    }

    std::shared_ptr<TextureManager> getTextureManager()
    {
        return mTextureManager;
    }

private:
    std::shared_ptr<Device> mDevice;
    std::shared_ptr<ShaderManager> mShaderManager;
    std::shared_ptr<EffectManager> mEffectManager;
    std::shared_ptr<RootSignatureManager> mRootSignatureManager;
    std::shared_ptr<PipelineStateManager> mPipelineStateManager;
    std::shared_ptr<TextureManager> mTextureManager;

};
