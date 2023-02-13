#include "framework.h"
#include "FrameData.h"
#include "Device.h"
#include "CommandList.h"
#include "World.h"
#include "Scene.h"
#include "Node.h"
#include "Mesh.h"
#include "Camera.h"
#include "CommandQueue.h"
#include "RenderGraph.h"
#include "RenderPass.h"

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
	mCommandLists.push_back(mclBeginFrame.get());

	// frame end command list
	mclEndFrame = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandLists.push_back(mclEndFrame.get());

	// render command list
	mclRender = device->createUniqueCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	mCommandLists.push_back(mclRender.get());
}

void FrameData::setWorld(std::shared_ptr<World> world)
{
	mWorld = world;
}

void FrameData::setRenderGraph(std::shared_ptr<RenderGraph> renderGraph)
{
	mRenderGraph = renderGraph;
}

void FrameData::setDirectCommandQueue(std::shared_ptr<CommandQueue> directCommandQueue)
{
	mDirectCommandQueue = directCommandQueue;
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

	//global settings
	mclBeginFrame->clearDepth(mDepthBufferView);

	// here, mclBeginFrame must be executed to ensure the order
	mDirectCommandQueue->executeCommandList(mclBeginFrame->commandList().Get());
}

void FrameData::renderFrame()
{
	mclRender->RSSetViewports(&mViewport);
	mclRender->RSSetScissorRects(&mScissorRect);
	mclRender->OMSetRenderTargets(&mBackBufferView, &mDepthBufferView);

	// multi-threading part
	mclRender->setPipelineState(mPipelineState.Get());
	mclRender->setGraphicsRootSignature(mRootSignature.Get());
	mclRender->IASetPrimitiveTopology();

	// Update the MVP matrix
	Camera *camera = mWorld->getCamera();
	DirectX::XMMATRIX viewProjMatrix = camera->modelViewProjectionMatrix();
	mclRender->setGraphicsRoot32BitConstants(0,
		sizeof(DirectX::XMMATRIX) / 4, 
		&viewProjMatrix,
		0);

	Scene* scene = mWorld->getScene();
	size_t meshCount = scene->nodeCount();

	for (size_t i = 0; i < meshCount; ++i)
	{
		Node* node = scene->node(i);
		DirectX::XMMATRIX worldMatrix = node->getWorldMatrix();
		mclRender->setGraphicsRoot32BitConstants(1,
			sizeof(DirectX::XMMATRIX) / 4,
			&worldMatrix,
			0);

		std::shared_ptr<Mesh> mesh = node->getMesh();

		mclRender->IASetVertexBuffers(0, 1, &(mesh->mVertexBuffer.mVertexBufferView));
		mclRender->IASetIndexBuffer(&(mesh->mIndexBuffer.mIndexBufferView));
		mclRender->drawIndexedInstanced(mesh->mIndexCount, 1, 0, 0, 0);
	}

	mDirectCommandQueue->executeCommandList(mclRender->commandList());
}

void FrameData::renderFrameA()
{
	mRenderGraph->execute(shared_from_this());
}

uint64_t FrameData::endFrame()
{
	mclEndFrame->transitionResource(mBackBuffer,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	return mDirectCommandQueue->executeCommandListAndSignal(mclEndFrame->commandList());
}

void FrameData::reset()
{
	for (CommandList* commandlist : mCommandLists)
	{
		commandlist->reset();
	}
}