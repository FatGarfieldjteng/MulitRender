#include "framework.h"
#include "FrameData.h"
#include "Device.h"
#include "CommandList.h"
#include "World.h"
#include "Scene.h"
#include "Node.h"
#include "Mesh.h"

FrameData::FrameData()
{

}

FrameData::~FrameData()
{

}

void FrameData::createCommandList(std::shared_ptr<Device> device)
{
	// frame begin command list
	mclBeginFrame = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mDX12CommandLists.push_back(mclBeginFrame->commandList().Get());
	mCommandLists.push_back(mclBeginFrame.get());

	// frame end command list
	mclEndFrame = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mDX12CommandLists.push_back(mclEndFrame->commandList().Get());
	mCommandLists.push_back(mclEndFrame.get());

	// render command list
	/*mRenderCommandList = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	mDX12CommandLists.push_back(mRenderCommandList->commandList().Get());
	mCommandLists.push_back(mRenderCommandList.get());*/

}

void FrameData::setWorld(std::shared_ptr<World> world)
{
	mWorld = world;
}

void FrameData::setViewport(const D3D12_VIEWPORT& viewport)
{
	mViewport = viewport;
}

void FrameData::setScissorRect(const D3D12_RECT& scissorRect)
{
	mScissorRect = scissorRect;
}

void FrameData::setBackBufferResource(ComPtr<ID3D12Resource> backBuffer)
{
	mBackBuffer = backBuffer;
}

void FrameData::setBackBufferView(const D3D12_CPU_DESCRIPTOR_HANDLE& backBufferView)
{
	mBackBufferView = backBufferView;
}

void FrameData::setDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilView)
{
	mDepthBufferView = depthStencilView;
}

void FrameData::setGraphicsRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
{
	mRootSignature = rootSignature;
}

void FrameData::setPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
{
	mPipelineState = pipelineState;
}

void FrameData::beginFrame()
{

	mclBeginFrame->transitionResource(mBackBuffer,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	mclBeginFrame->clearRTV(clearColor, mBackBufferView);
}

void FrameData::renderFrame()
{
	// global settings
	//auto dsv = mDSVHeap->GetCPUDescriptorHandleForHeapStart();
	//clearDepth(commandList, dsv);

	//commandList->SetPipelineState(mEffect->mPipelineState.Get());
	//commandList->SetGraphicsRootSignature(mEffect->mRootSignature.Get());

	//commandList->RSSetViewports(1, &mViewport);
	//commandList->RSSetScissorRects(1, &mScissorRect);

	//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtv = mSwapChain->getCurrentRTV();
	//commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	//// multi-threading part

	//// Update the MVP matrix
	//DirectX::XMMATRIX mvpMatrix = mCamera->modelViewProjectionMatrix();
	//commandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &mvpMatrix, 0);

	//Scene* scene = mWorld->getScene();
	//size_t meshCount = scene->nodeCount();

	//for (size_t i = 0; i < meshCount; ++i)
	//{
	//	Node* node = scene->node(i);
	//	std::shared_ptr<Mesh> mesh = node->getMesh();

	//	commandList->IASetVertexBuffers(0, 1, &(mesh->mVertexBuffer.mVertexBufferView));
	//	commandList->IASetIndexBuffer(&(mesh->mIndexBuffer.mIndexBufferView));
	//	commandList->DrawIndexedInstanced(mesh->mIndexCount, 1, 0, 0, 0);
	//}
}

void FrameData::endFrame()
{
	mclEndFrame->transitionResource(mBackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
}

void FrameData::reset()
{
	for (CommandList* commandlist : mCommandLists)
	{
		commandlist->reset();
	}
}

std::vector<ID3D12GraphicsCommandList2*>& FrameData::getCommandLists()
{
	return mDX12CommandLists;
}
