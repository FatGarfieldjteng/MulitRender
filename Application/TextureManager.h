#pragma once

#include <memory>
#include <map>
#include <string>

class Texture;

class TextureManager
{
public:

    TextureManager();

    ~TextureManager();

public:
    void addTexture(const std::string& ID, std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(const std::string& ID);


private:
    std::map<std::string, std::shared_ptr<Texture> > mIDToTexture;
};
