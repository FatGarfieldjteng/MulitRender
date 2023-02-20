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

    void setVS(ComPtr<ID3DBlob> VSBlob);
    void setPS(ComPtr<ID3DBlob> PSBlob);

    ComPtr<ID3DBlob> getVS();
    ComPtr<ID3DBlob> getPS();

protected:
    virtual void loadShader() {}

public:
    // vertex shader
    ComPtr<ID3DBlob> mVertexShaderBlob;
    
    // pixel shader
    ComPtr<ID3DBlob> mPixelShaderBlob;
};
