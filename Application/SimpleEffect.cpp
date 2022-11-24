#include "framework.h"
#include "d3dx12.h"
#include "SimpleEffect.h"
#include "Device.h"
#include "helper.h"
#include <DirectXMath.h>

SimpleEffect::SimpleEffect(std::shared_ptr<Device> device)
    :Effect(device)
{

}

SimpleEffect::~SimpleEffect()
{
    if (mInputLayout)
    {
        delete mInputLayout;
    }
}

void SimpleEffect::loadShader()
{
    // load vertex shader.
    ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &mVertexShaderBlob));

    // load pixel shader.
    ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &mPixelShaderBlob));
}

void SimpleEffect::createInputLayout()
{
    // create vertex input layout
    mInputLayout = new D3D12_INPUT_ELEMENT_DESC[2];
    mInputLayout[0] = { "POSITION", 
        0, 
        DXGI_FORMAT_R32G32B32_FLOAT, 
        0, 
        D3D12_APPEND_ALIGNED_ELEMENT, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 
        0 };

    mInputLayout[1] = { "COLOR", 
        0, 
        DXGI_FORMAT_R32G32B32_FLOAT, 
        0, 
        D3D12_APPEND_ALIGNED_ELEMENT, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 
        0 };
}

