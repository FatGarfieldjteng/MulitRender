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

    static std::wstring shaderPath()
    {
        wchar_t buff[MAX_PATH];
        GetModuleFileNameW(NULL, buff, MAX_PATH);
        std::wstring::size_type position = std::wstring(buff).find(L"MulitRender");
        
        std::wstring path = std::wstring(buff).substr(0, position);

        path += L"MulitRender\\Application\\Shader\\";

        return path;
    }

public:
    void build(std::shared_ptr<Device> device);

protected:
    virtual void loadShader() = 0;
    virtual void createRootSignature(std::shared_ptr<Device> device) = 0;
    virtual void createPipelineStateObject(std::shared_ptr<Device> device) = 0;

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
