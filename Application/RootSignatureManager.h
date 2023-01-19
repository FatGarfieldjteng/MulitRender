#pragma once

#include <map>
#include <string>

class RootSignature;

class RootSignatureManager
{
public:

    RootSignatureManager();

    ~RootSignatureManager();

public:
    void addRootSignature(const std::string& ID, RootSignature *rootSignature);

    RootSignature* rootSignature(const std::string& ID);

private:
    std::map<std::string, RootSignature*> mStringToRootSignature;
};
