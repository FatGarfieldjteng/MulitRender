#pragma once

#include <d3d12.h>
#include <memory>
#include <map>
#include <string>

class Device;

class RootSignatureManager
{
public:

    RootSignatureManager(std::shared_ptr<Device> device);

    ~RootSignatureManager();

public:
    ComPtr<ID3D12RootSignature> getRootSignature(const std::string& ID);
private:
    void createRootSignatures(std::shared_ptr<Device> device);
    void addRootSignature(const std::string& ID,
        ComPtr<ID3D12RootSignature> rootSignature);
    void createSimpleRootSignature(std::shared_ptr<Device> device);
    void createShadowRootSignature(std::shared_ptr<Device> device);

private:
    std::map<std::string, ComPtr<ID3D12RootSignature> > mIDToRootSignature;
};
