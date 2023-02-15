#include "framework.h"
#include "BeautyPass.h"
#include "Device.h"
#include "FrameData.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "TextureResource.h"
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

	for (size_t resourceIndex = 0; resourceIndex < mOutputResourceType.size(); ++resourceIndex)
	{
		switch (mOutputResourceType[resourceIndex])
		{
		case ResourceType_BackBuffer:
		{
			frameData->mclBeginFrame->transitionResource(mOutputResources[resourceIndex]->mResource,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			frameData->mclBeginFrame->clearRTV(clearColor, mOutputResources[resourceIndex]->mRTV);
		}
			break;
		case ResourceType_DepthStencil:
		{
			frameData->mclBeginFrame->clearDepth(mOutputResources[resourceIndex]->mDSV);
		}
			break;
		default:
			//error happens
			break;
		}
	}

	// here, mclBeginFrame must be executed to ensure the order
	frameData->mDirectCommandQueue->executeCommandList(frameData->mclBeginFrame->commandList().Get());

	// render, this part should be multithreading
	frameData->mclRender->RSSetViewports(&frameData->mViewport);
	frameData->mclRender->RSSetScissorRects(&frameData->mScissorRect);
	frameData->mclRender->OMSetRenderTargets(&mOutputResources[0]->mRTV, &mOutputResources[1]->mDSV);

	// multi-threading part
	frameData->mclRender->setPipelineState(mPipelineState.Get());
	frameData->mclRender->setGraphicsRootSignature(mRootSignature.Get());
	frameData->mclRender->IASetPrimitiveTopology();

	// Update the MVP matrix
	Camera* camera = frameData->mWorld->getCamera();
	DirectX::XMMATRIX viewProjMatrix = camera->modelViewProjectionMatrix();
	frameData->mclRender->setGraphicsRoot32BitConstants(0,
		sizeof(DirectX::XMMATRIX) / 4,
		&viewProjMatrix,
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

	// end render
	for (size_t resourceIndex = 0; resourceIndex < mOutputResourceType.size(); ++resourceIndex)
	{
		switch (mOutputResourceType[resourceIndex])
		{
		case ResourceType_BackBuffer:
		{
			frameData->mclEndFrame->transitionResource(mOutputResources[resourceIndex]->mResource,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT);
		}
		break;
		default:
			break;
		}
	}

	frameData->mDirectCommandQueue->executeCommandList(frameData->mclEndFrame->commandList());
}