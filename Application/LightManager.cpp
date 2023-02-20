#include "framework.h"
#include "LightManager.h"
#include "Light.h"

LightManager::LightManager()
{
	createLight();
}

LightManager::~LightManager()
{

}

void LightManager::createLight()
{
	createSimpleLight();
}

void LightManager::createSimpleLight()
{
	std::shared_ptr<Light> light = std::make_shared<Light>();

	light->mPosition = { 0.0f, 15.0f, -30.0f, 1.0f };
	light->mDirection = { 0.0, 0.0f, 1.0f, 0.0f };
	light->mFalloff = { 800.0f, 1.0f, 0.0f, 1.0f };
	light->mColor = { 0.7f, 0.7f, 0.7f, 1.0f };

	addLight("SimpleLight0", light);
}

void LightManager::addLight(const std::string& ID,
	std::shared_ptr<Light> light)
{
	mIDToLight[ID] = light;
}

std::shared_ptr<Light> LightManager::getLight(const std::string& ID)
{
	std::map<std::string, std::shared_ptr<Light>>::iterator it = mIDToLight.find(ID);

	if (it != mIDToLight.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}