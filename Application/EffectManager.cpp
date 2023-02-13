#include "framework.h"
#include "EffectManager.h"
#include "ShaderManager.h"
#include "Effect.h"
#include "helper.h"

EffectManager::EffectManager(std::shared_ptr<ShaderManager> shaderManager)
	:mShaderManager(shaderManager)
{
	createEffects();
}

EffectManager::~EffectManager()
{

}

void EffectManager::createEffects()
{
	createSimpleEffect();
	createShadowEffect();
}

void EffectManager::createSimpleEffect()
{
	std::shared_ptr<Effect> simpleEffect = std::make_shared<Effect>();
	simpleEffect->setVS(mShaderManager->getVS("SimpleVS"));
	simpleEffect->setPS(mShaderManager->getPS("SimplePS"));
	addEffect("SimpleEffect", simpleEffect);
}

void EffectManager::createShadowEffect()
{
	std::shared_ptr<Effect> shadowEffect = std::make_shared<Effect>();
	shadowEffect->setVS(mShaderManager->getVS("ShadowVS"));
	addEffect("ShadowEffect", shadowEffect);
}

void EffectManager::addEffect(const std::string& ID, std::shared_ptr<Effect> effect)
{
	mIDToEffect[ID] = effect;
}

std::shared_ptr<Effect>  EffectManager::getEffect(const std::string& ID)
{
	std::map<std::string, std::shared_ptr<Effect> >::iterator it = mIDToEffect.find(ID);

	if (it != mIDToEffect.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}