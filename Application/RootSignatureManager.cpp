#include "framework.h"
#include "RootSignatureManager.h"

RootSignatureManager::RootSignatureManager()
{

}

RootSignatureManager::~RootSignatureManager()
{

}

void RootSignatureManager::addRootSignature(const std::string& ID, RootSignature* rootSignature)
{
	mStringToRootSignature[ID] = rootSignature;
}

RootSignature* RootSignatureManager::rootSignature(const std::string& ID)
{
	std::map<std::string, RootSignature*>::iterator it = mStringToRootSignature.find(ID);

	if (it != mStringToRootSignature.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}