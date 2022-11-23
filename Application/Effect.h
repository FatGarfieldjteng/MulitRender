#pragma once
#include <d3d12.h>
#include <string>

class Effect
{
public:

    Effect();

    ~Effect();

    static std::string shaderPath()
    {
        return std::string("Shader");
    }
};
