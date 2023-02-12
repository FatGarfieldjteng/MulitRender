#pragma once

#include <map>
#include <string>
#include <memory>

class Effect;

class ShaderManager
{
public:

    static std::wstring shaderPath()
    {
        wchar_t buff[MAX_PATH];
        GetModuleFileNameW(NULL, buff, MAX_PATH);
        std::wstring::size_type position = std::wstring(buff).find(L"MulitRender");

        std::wstring path = std::wstring(buff).substr(0, position);

        path += L"MulitRender\\Application\\Shader\\";

        return path;
    }

    ShaderManager();

    ~ShaderManager();

public:
    void createShaders();

    void addVS(const std::string& ID, ComPtr<ID3DBlob>);
    void addPS(const std::string& ID, ComPtr<ID3DBlob>);

    ComPtr<ID3DBlob> getVS(const std::string& ID);
    ComPtr<ID3DBlob> getPS(const std::string& ID);

private:
    void createSimpleVS();
    void createSimplePS();

    void createShadowVS();

private:
    std::map<std::string, ComPtr<ID3DBlob> > mIDToVS;
    std::map<std::string, ComPtr<ID3DBlob> > mIDToPS;
};
