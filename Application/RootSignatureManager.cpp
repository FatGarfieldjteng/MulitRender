#include "framework.h"
#include "RootSignatureManager.h"
#include "Device.h"
#include "d3dx12.h"
#include "helper.h"

RootSignatureManager::RootSignatureManager(std::shared_ptr<Device> device)
{
	createRootSignatures(device);
}

RootSignatureManager::~RootSignatureManager()
{

}

void RootSignatureManager::createRootSignatures(std::shared_ptr<Device> device)
{
	// the color is as property of Vertex
	createSimpleRootSignature(device);

	// shadow root signature is the same as simple root signature
	createShadowRootSignature(device);
}

void RootSignatureManager::createSimpleRootSignature(std::shared_ptr<Device> device)
{
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// version is hard coded, 
	//in real scenario, featureData.HighestVersion could be 1_0 if hardware does not suppot 1_1
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	// only input assembler stage needs root signature
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
	rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	// no sampler
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// serialize root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	ComPtr<ID3D12RootSignature> rootSignature;
	device->CreateRootSignature(rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	addRootSignature("SimpleRootSignature", rootSignature);
}

void RootSignatureManager::createShadowRootSignature(std::shared_ptr<Device> device)
{
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// version is hard coded, 
	//in real scenario, featureData.HighestVersion could be 1_0 if hardware does not suppot 1_1
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	// only input assembler stage needs root signature
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[2] = {};
	rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	// no sampler
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// serialize root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	ComPtr<ID3D12RootSignature> rootSignature;
	device->CreateRootSignature(rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	addRootSignature("ShadowRootSignature", rootSignature);
}

void RootSignatureManager::addRootSignature(const std::string& ID, 
	ComPtr<ID3D12RootSignature> rootSignature)
{
	mIDToRootSignature[ID] = rootSignature;
}

ComPtr<ID3D12RootSignature> RootSignatureManager::getRootSignature(const std::string& ID)
{
	std::map<std::string, ComPtr<ID3D12RootSignature>>::iterator it = mIDToRootSignature.find(ID);

	if (it != mIDToRootSignature.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}