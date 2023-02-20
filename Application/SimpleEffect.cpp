#include "framework.h"
#include "d3dx12.h"
#include "SimpleEffect.h"
#include "Device.h"
#include "helper.h"
#include <DirectXMath.h>

SimpleEffect::SimpleEffect()
{

}

SimpleEffect::~SimpleEffect()
{
}

void SimpleEffect::loadShader()
{
	std::wstring path = shaderPath();

	std::wstring vertexShaderPath = path + L"SimpleVS.cso";

	std::wstring pixelShaderPath = path + L"SimplePS.cso";

    // load vertex shader.
    ThrowIfFailed(D3DReadFileToBlob(vertexShaderPath.c_str(), &mVertexShaderBlob));

    // load pixel shader.
    ThrowIfFailed(D3DReadFileToBlob(pixelShaderPath.c_str(), &mPixelShaderBlob));
}
