#pragma once

#include <memory>
#include <map>
#include <string>

class TextureResource;

class TextureManager
{
public:

    TextureManager();

    ~TextureManager();

public:
    void addTexture(const std::string& ID, std::shared_ptr<TextureResource> texture);
    std::shared_ptr<TextureResource> getTexture(const std::string& ID);


private:
    std::map<std::string, std::shared_ptr<TextureResource> > mIDToTexture;
};
