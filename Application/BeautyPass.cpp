#include "framework.h"
#include "BeautyPass.h"
#include "FrameData.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "TextureManager.h"
#include "TextureResource.h"
#include "Managers.h"
#include "CameraManager.h"
#include "HeapManager.h"
#include "Camera.h"
#include "World.h"
#include "Scene.h"
#include "Node.h"
#include "Mesh.h"

BeautyPass::BeautyPass()
{
}

BeautyPass::~BeautyPass()
{

}

void BeautyPass::render(FrameData* frameData)
{
	// begin render
	FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

	for (size_t resourceIndex = 0; resourceIndex < mOutputResources.size(); ++resourceIndex)
	{
		InOutReource &outResource = mOutputResources[resourceIndex];

		switch (outResource.type)
		{
		case ResourceType_BackBuffer:
		{
			frameData->mclBeginFrame->transitionResource(outResource.resource->mResource,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			frameData->mclBeginFrame->clearRTV(clearColor, outResource.resource->mRTV);
		}
			break;
		case ResourceType_DepthStencil:
		{
			frameData->mclBeginFrame->clearDepth(outResource.resource->mDSV);
		}
			break;
		default:
			//error happens
			break;
		}
	}

	// here, mclBeginFrame must be executed to ensure the order
	frameData->mDirectCommandQueue->executeCommandList(frameData->mclBeginFrame->commandList().Get());
	frameData->mclBeginFrame->resetCommandList();

	// render, this part should be multithreading
	frameData->mclRender->RSSetViewports(&frameData->mViewport);
	frameData->mclRender->RSSetScissorRects(&frameData->mScissorRect);

	InOutReource& backBufferResource = mOutputResources[0];
	InOutReource& depthStencilResource = mOutputResources[1];

	frameData->mclRender->OMSetRenderTargets(&backBufferResource.resource->mRTV, &depthStencilResource.resource->mDSV);

	// multi-threading part
	frameData->mclRender->setPipelineState(mPipelineState.Get());
	frameData->mclRender->setGraphicsRootSignature(mRootSignature.Get());

	// acquire two heaps
	ComPtr<ID3D12DescriptorHeap> SRVHeap = frameData->mManagers->getHeapManager()->getHeap("MainSRVHeap");
	ComPtr<ID3D12DescriptorHeap> samplerHeap = frameData->mManagers->getHeapManager()->getHeap("MainSamplerHeap");

	ID3D12DescriptorHeap* ppHeaps[] = { SRVHeap.Get(), samplerHeap.Get() };
	frameData->mclRender->setDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// shadow map

	// get proper shadow map GPU handle
	InOutReource& shadowMap = mInputResources[0];

	frameData->mclRender->setGraphicsRootDescriptorTable(3, shadowMap.resource->mSRV);

	// point sampler
	frameData->mclRender->setGraphicsRootDescriptorTable(4, samplerHeap->GetGPUDescriptorHandleForHeapStart());


	frameData->mclRender->IASetPrimitiveTopology();

	// Update the MVP matrix
	Camera* camera = frameData->mWorld->getCamera();
	DirectX::XMMATRIX viewProjMatrix = camera->modelViewProjectionMatrix();
	frameData->mclRender->setGraphicsRoot32BitConstants(0,
		sizeof(DirectX::XMMATRIX) / 4,
		&viewProjMatrix,
		0);

	// Update the MVP matrix for light camera
	std::shared_ptr<Camera> lightCamera = frameData->mManagers->getCameraManager()->getCamera("Light0Camera");
	DirectX::XMMATRIX lightViewProjMatrix = lightCamera->modelViewProjectionMatrix();
	frameData->mclRender->setGraphicsRoot32BitConstants(2,
		sizeof(DirectX::XMMATRIX) / 4,
		&lightViewProjMatrix,
		0);

	Scene* scene = frameData->mWorld->getScene();
	size_t meshCount = scene->nodeCount();

	for (size_t i = 0; i < meshCount; ++i)
	{
		Node* node = scene->node(i);
		DirectX::XMMATRIX worldMatrix = node->getWorldMatrix();
		frameData->mclRender->setGraphicsRoot32BitConstants(1,
			sizeof(DirectX::XMMATRIX) / 4,
			&worldMatrix,
			0);

		std::shared_ptr<Mesh> mesh = node->getMesh();

		frameData->mclRender->IASetVertexBuffers(0, 1, &(mesh->mVertexBuffer.mVertexBufferView));
		frameData->mclRender->IASetIndexBuffer(&(mesh->mIndexBuffer.mIndexBufferView));
		frameData->mclRender->drawIndexedInstanced(mesh->mIndexCount, 1, 0, 0, 0);
	}

	frameData->mDirectCommandQueue->executeCommandList(frameData->mclRender->commandList());

	frameData->mclRender->resetCommandList();

	// end render
	// transit shadow map for shadow pass to write

	frameData->mclEndFrame->transitionResource(shadowMap.resource->mResource,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);

	for (size_t resourceIndex = 0; resourceIndex < mOutputResources.size(); ++resourceIndex)
	{
		InOutReource& outResource = mOutputResources[resourceIndex];

		switch (outResource.type)
		{
		case ResourceType_BackBuffer:
		{
			frameData->mclEndFrame->transitionResource(outResource.resource->mResource,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT);
		}
		break;
		default:
			break;
		}
	}

	frameData->mDirectCommandQueue->executeCommandList(frameData->mclEndFrame->commandList());
	frameData->mclEndFrame->resetCommandList();
}