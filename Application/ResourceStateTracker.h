#pragma once

#include <d3d12.h>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>

class CommandList;
class Resource;

class ResourceStateTracker
{
public:

    ResourceStateTracker();

    ~ResourceStateTracker();

public:
    void resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

    void transitionResource(ID3D12Resource* resource, 
        D3D12_RESOURCE_STATES stateAfter, 
        UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    void transitionResource(const Resource& resource, 
        D3D12_RESOURCE_STATES stateAfter, 
        UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    void UAVBarrier(const Resource* resource = nullptr);

    void aliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);

    uint32_t flushPendingResourceBarriers(CommandList& commandList);

    void flushResourceBarriers(CommandList& commandList);

    void commitFinalResourceStates();

    void reset();

    static void lock();

    static void unlock();

    static void addGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

    static void removeGlobalResourceState(ID3D12Resource* resource);

private:
    using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

    ResourceBarriers mPendingResourceBarriers;

    ResourceBarriers mResourceBarriers;

    struct ResourceState
    {
        // Initialize all of the subresources within a resource to the given state.
        explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
            : State(state)
        {}

        void setSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state)
        {
            if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                State = state;
                SubresourceState.clear();
            }
            else
            {
                SubresourceState[subresource] = state;
            }
        }


        D3D12_RESOURCE_STATES getSubresourceState(UINT subresource) const
        {
            D3D12_RESOURCE_STATES state = State;
            const auto iter = SubresourceState.find(subresource);
            if (iter != SubresourceState.end())
            {
                state = iter->second;
            }
            return state;
        }

        D3D12_RESOURCE_STATES State;
        std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
    };

    using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

    ResourceStateMap mFinalResourceState;
    static ResourceStateMap msGlobalResourceState;
    static std::mutex msGlobalMutex;
    static bool msIsLocked;
};
