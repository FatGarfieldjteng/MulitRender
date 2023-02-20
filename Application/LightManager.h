#pragma once

#include <memory>
#include <map>
#include <string>

class Light;

class LightManager
{
public:

    LightManager();

    ~LightManager();

public:
    std::shared_ptr<Light> getLight(const std::string& ID);
private:
    void createLight();
    void addLight(const std::string& ID,
        std::shared_ptr<Light> light);
    void createSimpleLight();
    
private:
    std::map<std::string, std::shared_ptr<Light> > mIDToLight;
};
