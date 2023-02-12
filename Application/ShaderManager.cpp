#include "framework.h"
#include "ShaderManager.h"
#include "helper.h"

ShaderManager::ShaderManager()
{
	createShaders();
}

ShaderManager::~ShaderManager()
{

}

void ShaderManager::createShaders()
{
	createSimpleVS();
	createSimplePS();

	createShadowVS();
}

void ShaderManager::createSimpleVS()
{
	std::wstring path = shaderPath();

	std::wstring vertexShaderPath = path + L"SimpleVS.cso";

	ComPtr<ID3DBlob> vsBlob;

	// load vertex shader.
	ThrowIfFailed(D3DReadFileToBlob(vertexShaderPath.c_str(), &vsBlob));

	addVS("SimpleVS", vsBlob);
}

void ShaderManager::createSimplePS()
{
	std::wstring path = shaderPath();

	std::wstring pixelShaderPath = path + L"SimplePS.cso";

	ComPtr<ID3DBlob> psBlob;

	// load pixel shader.
	ThrowIfFailed(D3DReadFileToBlob(pixelShaderPath.c_str(), &psBlob));

	addPS("SimplePS", psBlob);
}

void ShaderManager::createShadowVS()
{
	std::wstring path = shaderPath();

	std::wstring vertexShaderPath = path + L"ShadowVS.cso";

	ComPtr<ID3DBlob> vsBlob;

	// load vertex shader.
	ThrowIfFailed(D3DReadFileToBlob(vertexShaderPath.c_str(), &vsBlob));

	addVS("ShadowVS", vsBlob);
}

void ShaderManager::addVS(const std::string& ID, ComPtr<ID3DBlob> vsBlob)
{
	mIDToVS[ID] = vsBlob;
}

void ShaderManager::addPS(const std::string& ID, ComPtr<ID3DBlob> psBlob)
{
	mIDToPS[ID] = psBlob;
}

ComPtr<ID3DBlob> ShaderManager::getVS(const std::string& ID)
{
	std::map<std::string, ComPtr<ID3DBlob> >::iterator it = mIDToVS.find(ID);

	if (it != mIDToVS.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}

ComPtr<ID3DBlob> ShaderManager::getPS(const std::string& ID)
{
	std::map<std::string, ComPtr<ID3DBlob> >::iterator it = mIDToPS.find(ID);

	if (it != mIDToPS.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}