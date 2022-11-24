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

void SimpleEffect::createRootSignature()
{
	// Create a root signature.
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// version is hard coded, 
	//in real scenario, featureData.HighestVersion could be 1_0 if hardware does not suppot 1_1
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	// only input assembler stage needs root signature
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
	rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	// no sampler
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// serialize root signature.
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
	// Create the root signature.
	mDevice->CreateRootSignature(rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

void SimpleEffect::createPipelineStateObject()
{
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	pipelineStateStream.pRootSignature = mRootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(mVertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(mPixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};
	
	mDevice->createPipelineStateObject(&pipelineStateStreamDesc, IID_PPV_ARGS(&mPipelineState));
}

