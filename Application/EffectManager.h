#pragma once

#include <map>
#include <string>
#include <memory>

class Effect;
class ShaderManager;

class EffectManager
{
public:

    EffectManager(std::shared_ptr<ShaderManager> shaderManager);

    ~EffectManager();

public:
    void createEffects();

    void addEffect(const std::string& ID, std::shared_ptr<Effect> effect);

    std::shared_ptr<Effect> getEffect(const std::string& ID);

private:
    void createSimpleEffect();
    void createShadowEffect();

private:
    std::map<std::string, std::shared_ptr<Effect> > mIDToEffect;
    std::shared_ptr<ShaderManager> mShaderManager;
};
