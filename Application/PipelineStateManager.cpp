#include "framework.h"
#include "PipelineStateManager.h"
#include "RootSignatureManager.h"
#include "EffectManager.h"
#include "Effect.h"
#include "d3dx12.h"
#include "Device.h"
#include "helper.h"

PipelineStateManager::PipelineStateManager(std::shared_ptr<Device> device,
	std::shared_ptr <RootSignatureManager> rootSignatureManager,
	std::shared_ptr <EffectManager> effectManager)
	:mDevice(device), 
	mRootSignatureManager(rootSignatureManager),
	mEffectManager(effectManager)
{
	createPipelineStates();
}

PipelineStateManager::~PipelineStateManager()
{

}

void PipelineStateManager::createPipelineStates()
{
	createSimplePipelineState();
	createShadowPipelineState();
}

void PipelineStateManager::createSimplePipelineState()
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

	ComPtr<ID3D12PipelineState> pipelineState;

	std::shared_ptr<Effect> effect = mEffectManager->getEffect("SimpleEffect");

	pipelineStateStream.pRootSignature = mRootSignatureManager->getRootSignature("SimpleRootSignature").Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(effect->getVS().Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(effect->getPS().Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	mDevice->createPipelineStateObject(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState));

	addPipelineState("SimplePipelineState", pipelineState);
}

void PipelineStateManager::createShadowPipelineState()
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

	ComPtr<ID3D12PipelineState> pipelineState;

	std::shared_ptr<Effect> effect = mEffectManager->getEffect("ShadowEffect");

	pipelineStateStream.pRootSignature = mRootSignatureManager->getRootSignature("ShadowRootSignature").Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(effect->getVS().Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE();
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	mDevice->createPipelineStateObject(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState));

	addPipelineState("ShadowPipelineState", pipelineState);
}

void PipelineStateManager::addPipelineState(const std::string& ID, ComPtr<ID3D12PipelineState> pipelineState)
{
	mStringToPipelineState[ID] = pipelineState;
}

ComPtr<ID3D12PipelineState> PipelineStateManager::getPipelineState(const std::string& ID)
{
	std::map<std::string, ComPtr<ID3D12PipelineState>>::iterator it = mStringToPipelineState.find(ID);

	if (it != mStringToPipelineState.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}