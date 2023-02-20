#pragma once

#include <memory>
#include <map>
#include <string>

class Camera;

class CameraManager
{
public:

    CameraManager();

    ~CameraManager();

public:
    std::shared_ptr<Camera> getCamera(const std::string& ID);
    void addCamera(const std::string& ID, std::shared_ptr<Camera> camera);
   
private:
    std::map<std::string, std::shared_ptr<Camera> > mIDToCamera;
};
