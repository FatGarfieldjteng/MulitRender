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
       // resetCommandAllocator();
        
    }

    void resetCommandList() const
    {
        mCommandList->Reset(mCommandAllocator.Get(), nullptr);
    }

    void resetCommandAllocator()
    {
        mCommandAllocator->Reset();
    }

    void descriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);

public:
    std::shared_ptr<Texture> loadTextureFromFile(const std::wstring& fileName, bool sRGB = false);

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

    // transition barrier
    void transitionBarrier(const std::shared_ptr<GraphicsResource>& resource,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        bool flushBarriers = false);

    void transitionBarrier(ComPtr<ID3D12Resource> resource, 
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, 
        bool flushBarriers = false);

    void flushResourceBarriers();

    void UAVBarrier(const GraphicsResource& resource, bool flushBarriers = false);

    void AliasingBarrier(const GraphicsResource& beforeResource, const GraphicsResource& afterResource, bool flushBarriers = false);

    void copyResource(ComPtr<ID3D12Resource> dstRes,
        ComPtr<ID3D12Resource> srcRes);

    void setGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex,
        size_t sizeInBytes,
        const void* bufferData);

 /*   void setShaderResourceView(uint32_t rootParameterIndex,
        uint32_t descriptorOffset,
        const GraphicsResource& resource,
        D3D12_RESOURCE_STATES stateAfter,
        UINT firstSubresource,
        UINT numSubresources,
        const D3D12_SHADER_RESOURCE_VIEW_DESC* srv);*/

    void draw(uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t startVertex,
        uint32_t startInstance);

    void drawIndexed(uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t startIndex,
        int32_t baseVertex,
        uint32_t startInstance);
private:
    void bindDescriptorHeaps();

    void trackResource(ComPtr<ID3D12Object> object);
    void trackResource(const std::shared_ptr<GraphicsResource>& res);

    void copyTextureSubresource(const std::shared_ptr<Texture>& texture,
        uint32_t firstSubresource,
        uint32_t numSubresources,
        D3D12_SUBRESOURCE_DATA* subresourceData);

    void generateMips(const std::shared_ptr<Texture>& texture);
private:
    std::shared_ptr<Device> mDevice;

    std::unique_ptr<UploadBuffer> mUploadBuffer;

    // command list, backend command allocator, command list type
    D3D12_COMMAND_LIST_TYPE mCommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> mCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mCommandAllocator;

    // 
    std::shared_ptr<CommandList> mComputeCommandList;

    std::unique_ptr<ViewManager> mViewManagers[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
    ID3D12DescriptorHeap* mDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    // resource state tracker
    std::unique_ptr<ResourceStateTracker> mResourceStateTracker;

    std::vector<ComPtr<ID3D12Object>> mTrackedObjects;
};