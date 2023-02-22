#include "framework.h"
#include "CommandList.h"
#include "Device.h"
#include "UploadBuffer.h"
#include "ViewManager.h"
#include "GraphicsResource.h"
#include "ResourceStateTracker.h"
#include "Texture.h"
#include "helper.h"
#include <DirectXTex.h>
#include <filesystem>

namespace fs = std::filesystem;

std::map<std::wstring, ID3D12Resource*> CommandList::msTextureCache;
std::mutex                              CommandList::msTextureCacheMutex;

CommandList::CommandList(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type)
    : mDevice(device)
    , mCommandListType(type)
{
    mCommandAllocator = mDevice->createCommandAllocator(mCommandListType);

    mCommandList = mDevice->createCommandList(mCommandAllocator, mCommandListType);

    mUploadBuffer = std::make_unique<UploadBuffer>(mDevice);

    mSingle = true;
}

CommandList::CommandList(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type, int count)
    : mDevice(device)
    , mCommandListType(type)
{
    for (int i = 0; i < count; ++i)
    {
        ComPtr<ID3D12CommandAllocator> commandAllocator = mCommandAllocators.emplace_back(mDevice->createCommandAllocator(mCommandListType));
        mCommandLists.emplace_back(mDevice->createCommandList(commandAllocator, mCommandListType));
    }

    mSingle = false;
}

CommandList::~CommandList()
{}

void CommandList::transitionBarrier(ComPtr<ID3D12Resource> resource, 
    D3D12_RESOURCE_STATES stateAfter,
    UINT subresource)
{
    if (resource)
    {
        // The "before" state is not important. It will be resolved by the resource state tracker.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter,
            subresource);
    }
}

void CommandList::transitionResource(ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState,
    D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    mCommandList->ResourceBarrier(1, &barrier);
}

void CommandList::clearRTV(FLOAT* clearValue, D3D12_CPU_DESCRIPTOR_HANDLE rtv)
{
    mCommandList->ClearRenderTargetView(rtv, clearValue, 0, nullptr);
}

void CommandList::clearDepth(D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    mCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void CommandList::setPipelineState(ID3D12PipelineState* pPipelineState)
{
    mCommandList->SetPipelineState(pPipelineState);
}

void CommandList::setGraphicsRootSignature(ID3D12RootSignature* pRootSignature)
{
    mCommandList->SetGraphicsRootSignature(pRootSignature);
}

void CommandList::setDescriptorHeaps(UINT NumDescriptorHeaps,
    ID3D12DescriptorHeap* const* ppDescriptorHeaps)
{
    mCommandList->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
}

void CommandList::setGraphicsRootDescriptorTable(UINT RootParameterIndex,
    D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
{
    mCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
}

void CommandList::RSSetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports)
{
    mCommandList->RSSetViewports(NumViewports, pViewports);
}

void CommandList::RSSetScissorRects(const D3D12_RECT* pRects, UINT NumRects)
{
    mCommandList->RSSetScissorRects(NumRects, pRects);
}


void CommandList::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
    mCommandList->IASetPrimitiveTopology(PrimitiveTopology);
}

void CommandList::OMSetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor,
    UINT NumRenderTargetDescriptors,
    BOOL RTsSingleHandleToDescriptorRange)
{
    mCommandList->OMSetRenderTargets(NumRenderTargetDescriptors, 
        pRenderTargetDescriptors, 
        RTsSingleHandleToDescriptorRange, 
        pDepthStencilDescriptor);
}

void CommandList::setGraphicsRoot32BitConstants(UINT RootParameterIndex,
    UINT Num32BitValuesToSet,
    const void* pSrcData,
    UINT DestOffsetIn32BitValues)
{
    mCommandList->SetGraphicsRoot32BitConstants(RootParameterIndex,
        Num32BitValuesToSet, 
        pSrcData, 
        DestOffsetIn32BitValues);
}

void CommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
{
    mCommandList->IASetIndexBuffer(pView);
}

void CommandList::IASetVertexBuffers(UINT StartSlot,
    UINT NumViews,
    const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
    mCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void CommandList::drawIndexedInstanced(UINT IndexCountPerInstance,
    UINT InstanceCount,
    UINT StartIndexLocation,
    INT BaseVertexLocation,
    UINT StartInstanceLocation)
{
    mCommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void CommandList::setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, 
    size_t sizeInBytes, 
    const void* bufferData)
{
    // Constant buffers must be 256-byte aligned.
    UploadBuffer::Allocation allocation = mUploadBuffer->allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    memcpy(allocation.CPUAddress, bufferData, sizeInBytes);

    mCommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, allocation.GPUAddress);
}