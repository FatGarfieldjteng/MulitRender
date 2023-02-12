#include "framework.h"
#include "Managers.h"
#include "ShaderManager.h"
#include "EffectManager.h"
#include "RootSignatureManager.h"
#include "PipelineStateManager.h"

Managers::Managers(std::shared_ptr<Device> device)
	:mDevice(device)
{

}

Managers::~Managers()
{

}

void Managers::createManagers()
{
	mShaderManager = std::make_shared<ShaderManager>();
	mEffectManager = std::make_shared<EffectManager>(mShaderManager);
	mRootSignatureManager = std::make_shared<RootSignatureManager>(mDevice);
	mPipelineStateManager = std::make_shared<PipelineStateManager>(mDevice, mRootSignatureManager, mEffectManager);
}