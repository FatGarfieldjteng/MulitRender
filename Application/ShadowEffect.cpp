#include "framework.h"
#include "d3dx12.h"
#include "ShadowEffect.h"
#include "Device.h"
#include "helper.h"
#include <DirectXMath.h>

ShadowEffect::ShadowEffect()
{

}

ShadowEffect::~ShadowEffect()
{
    
}

void ShadowEffect::loadShader()
{
	std::wstring path = shaderPath();

	std::wstring vertexShaderPath = path + L"ShadowVS.cso";

    // load vertex shader.
    ThrowIfFailed(D3DReadFileToBlob(vertexShaderPath.c_str(), &mVertexShaderBlob));

    // no pixel shader needed, this effect is used in depth only pass
}
