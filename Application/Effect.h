#pragma once
#include <d3d12.h>
#include <string>
#include <memory>

class Device;

class Effect
{
public:

    Effect(std::shared_ptr<Device> device);

    ~Effect();

    static std::string shaderPath()
    {
        return std::string("Shader");
    }

public:
    void init();

protected:
    virtual void loadShader() = 0;
    virtual void createInputLayout() = 0;

protected:
    std::shared_ptr<Device> mDevice;

public:
    // vertex shader
    ComPtr<ID3DBlob> mVertexShaderBlob;
    
    // pixel shader
    ComPtr<ID3DBlob> mPixelShaderBlob;
    
    // input layout
    D3D12_INPUT_ELEMENT_DESC* mInputLayout = nullptr;
};
