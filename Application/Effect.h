#pragma once
#include <d3d12.h>
#include <string>
#include <memory>

class Device;

class Effect
{
public:

    Effect();

    ~Effect();

    static std::string shaderPath()
    {
        return std::string("Shader");
    }

public:
    void build(std::shared_ptr<Device> device);

protected:
    virtual void loadShader() = 0;
    virtual void createRootSignature(std::shared_ptr<Device> device) = 0;
    virtual void createPipelineStateObject(std::shared_ptr<Device> device) = 0;

protected:
    std::shared_ptr<Device> mDevice;

public:
    // vertex shader
    ComPtr<ID3DBlob> mVertexShaderBlob;
    
    // pixel shader
    ComPtr<ID3DBlob> mPixelShaderBlob;
    
    // input layout
    D3D12_INPUT_ELEMENT_DESC* mInputLayout = nullptr;

    // root signature
    ComPtr<ID3D12RootSignature> mRootSignature;

    // pipeline state object.
    ComPtr<ID3D12PipelineState> mPipelineState;
};
