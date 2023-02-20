#include "framework.h"
#include "Managers.h"
#include "ShaderManager.h"
#include "EffectManager.h"
#include "RootSignatureManager.h"
#include "PipelineStateManager.h"
#include "TextureManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "HeapManager.h"

Managers::Managers(std::shared_ptr<Device> device)
	:mDevice(device)
{
	createManagers();
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
	mTextureManager = std::make_shared<TextureManager>();
	mLightManager = std::make_shared<LightManager>();
	mCameraManager = std::make_shared<CameraManager>();
	mHeapManager = std::make_shared<HeapManager>();
}