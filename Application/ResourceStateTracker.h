#pragma once

#include <d3d12.h>
#include <mutex>
#include <vector>
#include <map>
#include <unordered_map>

class CommandList;
class GraphicsResource;

class ResourceStateTracker
{
public:

    ResourceStateTracker();

    ~ResourceStateTracker();

public:

    // only take into account barrier.Transition.StateAfter
    // State before is acquired from mFinalResourceState
    // if the info can be acquired from mFinalResourceState
    void resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

    void transitionResource(ID3D12Resource* resource, 
        D3D12_RESOURCE_STATES stateAfter, 
        UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    void transitionResource(const GraphicsResource& resource,
        D3D12_RESOURCE_STATES stateAfter, 
        UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    void UAVBarrier(const GraphicsResource* resource = nullptr);

    void AliasBarrier(const GraphicsResource* resourceBefore = nullptr, const GraphicsResource* resourceAfter = nullptr);

    uint32_t flushPendingResourceBarriers(CommandList& commandList);

    void flushResourceBarriers(CommandList& commandList);

    // merge transition state map entry from mFinalResourceState to msGlobalResourceState
    void commitFinalResourceStates();

    void reset();

    // lock msGlobalMutex
    static void lock();

    // unlock msGlobalMutex
    static void unlock();

    static void addGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

    static void removeGlobalResourceState(ID3D12Resource* resource);

private:
    using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

    ResourceBarriers mPendingResourceBarriers;

    // stores aliasing and UAV barrier, transition barrier is not managed by mResourceBarriers
    ResourceBarriers mResourceBarriers;

    // ResourceState stores states of subresources
    // if no subresource presents, an overall state is stored in State 
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

        // map sub resource ID to resource state
        std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
    };


    // resource->ResouceState
    using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

    ResourceStateMap mFinalResourceState;
    static ResourceStateMap msGlobalResourceState;
    static std::mutex msGlobalMutex;
    static bool msIsLocked;
};
