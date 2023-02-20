#include "framework.h"
#include "CameraManager.h"
#include "Camera.h"

CameraManager::CameraManager()
{

}

CameraManager::~CameraManager()
{

}

void CameraManager::addCamera(const std::string& ID,
	std::shared_ptr<Camera> camera)
{
	mIDToCamera[ID] = camera;
}

std::shared_ptr<Camera> CameraManager::getCamera(const std::string& ID)
{
	std::map<std::string, std::shared_ptr<Camera>>::iterator it = mIDToCamera.find(ID);

	if (it != mIDToCamera.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}