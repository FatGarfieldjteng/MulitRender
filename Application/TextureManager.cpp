#include "framework.h"
#include "TextureManager.h"
#include "Texture.h"

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{

}

void TextureManager::addTexture(const std::string& ID, std::shared_ptr<Texture> texture)
{
	mIDToTexture[ID] = texture;
}

std::shared_ptr<Texture> TextureManager::getTexture(const std::string& ID)
{
	std::map<std::string, std::shared_ptr<Texture> > ::iterator it = mIDToTexture.find(ID);

	if (it != mIDToTexture.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}