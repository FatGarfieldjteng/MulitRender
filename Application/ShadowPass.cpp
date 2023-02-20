#include "framework.h"
#include "ShadowPass.h"
#include "FrameData.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "TextureResource.h"
#include "Managers.h"
#include "CameraManager.h"
#include "Camera.h"
#include "World.h"
#include "Scene.h"
#include "Node.h"
#include "Mesh.h"

#include <cassert>

ShadowPass::ShadowPass()
{
}

ShadowPass::~ShadowPass()
{
	
}

void ShadowPass::render(FrameData* frameData)
{
	// begin render
	// check if there is a out resource that acts as shadow map

	assert(mOutputResources.size() == 1);

	// clear shadow map
	InOutReource& outResource = mOutputResources[0];
	frameData->mclBeginFrame->clearDepth(outResource.resource->mDSV);
	
	// here, mclBeginFrame must be executed to ensure the order
	frameData->mDirectCommandQueue->executeCommandList(frameData->mclBeginFrame->commandList().Get());

	// render, this part should be multithreading
	frameData->mclRender->RSSetViewports(&frameData->mViewport);
	frameData->mclRender->RSSetScissorRects(&frameData->mScissorRect);
	
	frameData->mclRender->OMSetRenderTargets(nullptr, &outResource.resource->mDSV, 0);

	// multi-threading part
	frameData->mclRender->setPipelineState(mPipelineState.Get());
	frameData->mclRender->setGraphicsRootSignature(mRootSignature.Get());
	frameData->mclRender->IASetPrimitiveTopology();

	// Update the MVP matrix for light camera
	std::shared_ptr<Camera> lightCamera = frameData->mManagers->getCameraManager()->getCamera("Light0Camera");
	DirectX::XMMATRIX lightViewProjMatrix = lightCamera->modelViewProjectionMatrix();
	frameData->mclRender->setGraphicsRoot32BitConstants(0,
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

	// end render is not necessary, because no need to transit render target's state
}