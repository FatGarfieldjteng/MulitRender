#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <map>

class Device;
class ViewManager;
class UploadBuffer;
class GraphicsResource;
class ResourceStateTracker;
class Texture;

class CommandList
{
public:
    CommandList(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type);
    CommandList(std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type, int count);
    virtual ~CommandList();

public:

    D3D12_COMMAND_LIST_TYPE commandListType() const
    {
        return mCommandListType;
    }

    ComPtr<ID3D12GraphicsCommandList2> commandList() const
    {
        return mCommandList;
    }

    void resetAll()
    {
        resetCommandList();
    }

    void resetCommandList() const
    {
        mCommandList->Reset(mCommandAllocator.Get(), nullptr);
    }

    void resetCommandAllocator()
    {
        mCommandAllocator->Reset();
    }

private:
    static std::map<std::wstring, ID3D12Resource*> msTextureCache;
    static std::mutex                              msTextureCacheMutex;

public:
    void transitionResource(ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState,
        D3D12_RESOURCE_STATES afterState);

    void clearRTV(FLOAT* clearValue, D3D12_CPU_DESCRIPTOR_HANDLE rtv);

    void clearDepth(D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    void setPipelineState(ID3D12PipelineState* pPipelineState);
    void setGraphicsRootSignature(ID3D12RootSignature* pRootSignature);

    void setDescriptorHeaps(UINT NumDescriptorHeaps,
        ID3D12DescriptorHeap* const* ppDescriptorHeaps);

    void setGraphicsRootDescriptorTable(UINT RootParameterIndex,
        D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor);

    void RSSetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports = 1);
    void RSSetScissorRects(const D3D12_RECT* pRects, UINT NumRects = 1);

    void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    void OMSetRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor,
        UINT NumRenderTargetDescriptors = 1,
        BOOL RTsSingleHandleToDescriptorRange = FALSE
        );

    void setGraphicsRoot32BitConstants(UINT RootParameterIndex,
        UINT Num32BitValuesToSet,
        const void* pSrcData,
        UINT DestOffsetIn32BitValues);

    void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView);

    void IASetVertexBuffers(UINT StartSlot,
        UINT NumViews,
        const D3D12_VERTEX_BUFFER_VIEW* pViews);

    void drawIndexedInstanced(UINT IndexCountPerInstance,
        UINT InstanceCount,
        UINT StartIndexLocation,
        INT BaseVertexLocation,
        UINT StartInstanceLocation);

    void transitionBarrier(ComPtr<ID3D12Resource> resource, 
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);


    void setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex,
        size_t sizeInBytes,
        const void* bufferData);


private:
    std::shared_ptr<Device> mDevice;

    std::unique_ptr<UploadBuffer> mUploadBuffer;

    // command list, backend command allocator, command list type
    D3D12_COMMAND_LIST_TYPE mCommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;

    std::unique_ptr<ViewManager> mViewManagers[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    ID3D12DescriptorHeap* mDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};