#include "framework.h"
#include "ResourceStateTracker.h"
#include "GraphicsResource.h"
#include "CommandList.h"
#include "d3dx12.h"

// Static definitions.
std::mutex ResourceStateTracker::msGlobalMutex;
bool ResourceStateTracker::msIsLocked = false;
ResourceStateTracker::ResourceStateMap ResourceStateTracker::msGlobalResourceState;

ResourceStateTracker::ResourceStateTracker()
{

}

ResourceStateTracker::~ResourceStateTracker()
{

}

void ResourceStateTracker::resourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
{
    if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
    {
        const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

        // try to find the entry in mFinalResourceState using resource as key
        // if found, thus the resouce has been used in this command list,
        // and its previous state is stored in mFinalResourceState
        const auto iter = mFinalResourceState.find(transitionBarrier.pResource);

        // found in mFinalResourceState
        if (iter != mFinalResourceState.end())
        {
            auto& resourceState = iter->second;

            // all sub resources
            if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                !resourceState.SubresourceState.empty())
            {
                // First transition all of the subresources if they are different than the StateAfter.
                for (auto subresourceState : resourceState.SubresourceState)
                {
                    if (transitionBarrier.StateAfter != subresourceState.second)
                    {
                        D3D12_RESOURCE_BARRIER newBarrier = barrier;
                        newBarrier.Transition.Subresource = subresourceState.first;
                        newBarrier.Transition.StateBefore = subresourceState.second;
                        mResourceBarriers.push_back(newBarrier);
                    }
                }
            }
            else
            {
                // individual sub resource
                auto finalState = resourceState.getSubresourceState(transitionBarrier.Subresource);
                if (transitionBarrier.StateAfter != finalState)
                {
                    // Push a new transition barrier with the correct before state.
                    D3D12_RESOURCE_BARRIER newBarrier = barrier;
                    newBarrier.Transition.StateBefore = finalState;
                    mResourceBarriers.push_back(newBarrier);
                }
            }
        }
        // the resource is never used by this command list before, thus add it to pending resource barrier,
        // commit pending resource barrier need to compare to global resource barrier
        else
        {
            // pending barriers will be resolved
            // before the command list is executed on the command queue.
            mPendingResourceBarriers.push_back(barrier);
        }

        // since this resource is used by this command list, add an entry to mFinalResourceState
        mFinalResourceState[transitionBarrier.pResource].setSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
    }
    else
    {
        // non-transitionBarrier, simply add entry to mResourceBarriers
        mResourceBarriers.push_back(barrier);
    }
}


void ResourceStateTracker::transitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
    if (resource)
    {
        // the before state of transition is not relevant
        resourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource));
    }
}

void ResourceStateTracker::transitionResource(const GraphicsResource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource)
{
    transitionResource(resource.mResource.Get(), stateAfter, subResource);
}

void ResourceStateTracker::UAVBarrier(const GraphicsResource* resource)
{
    ID3D12Resource* pResource = nullptr;

    if (resource)
    {
        pResource = resource->mResource.Get();
    }

    resourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
}

void ResourceStateTracker::AliasBarrier(const GraphicsResource* resourceBefore, const GraphicsResource* resourceAfter)
{
    ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->mResource.Get() : nullptr;
    ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->mResource.Get() : nullptr;

    resourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
}

void ResourceStateTracker::flushResourceBarriers(CommandList& commandList)
{
    UINT numBarriers = static_cast<UINT>(mResourceBarriers.size());
    if (numBarriers > 0)
    {
        auto dxCommandList = commandList.commandList();
        dxCommandList->ResourceBarrier(numBarriers, mResourceBarriers.data());
        mResourceBarriers.clear();
    }
}


uint32_t ResourceStateTracker::flushPendingResourceBarriers(CommandList& commandList)
{
    // must lock global resource barrier map
    assert(msIsLocked);

    // resolve array
    ResourceBarriers resourceBarriers;
    
    resourceBarriers.reserve(mPendingResourceBarriers.size());

    for (auto& pendingBarrier : mPendingResourceBarriers)
    {
        // this is a must, should use assert to validate this condition
        if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
        {
            auto pendingTransition = pendingBarrier.Transition;

            // find resource's global state
            const auto& iter = msGlobalResourceState.find(pendingTransition.pResource);
            
            // should be found, use assert for validation
            if (iter != msGlobalResourceState.end())
            {
                auto& globalResourceState = iter->second;

                if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                    !globalResourceState.SubresourceState.empty())
                {
                    // transition all subresources
                    for (auto& globalSubresourceState : globalResourceState.SubresourceState)
                    {
                        if (pendingTransition.StateAfter != globalSubresourceState.second)
                        {
                            D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
                            newBarrier.Transition.Subresource = globalSubresourceState.first;
                            newBarrier.Transition.StateBefore = globalSubresourceState.second;
                            resourceBarriers.push_back(newBarrier);
                        }
                    }
                }
                else
                {
                    // individual transition
                    auto globalState = (iter->second).getSubresourceState(pendingTransition.Subresource);
                    if (pendingTransition.StateAfter != globalState)
                    {
                        pendingBarrier.Transition.StateBefore = globalState;
                        resourceBarriers.push_back(pendingBarrier);
                    }
                }
            } // if (iter != msGlobalResourceState.end())
        } // if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
    } // for (auto& pendingBarrier : mPendingResourceBarriers)

    UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
    if (numBarriers > 0)
    {
        auto dxCommandList = commandList.commandList();
        dxCommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
    }

    mPendingResourceBarriers.clear();

    return numBarriers;
}

void ResourceStateTracker::commitFinalResourceStates()
{
    assert(msIsLocked);

    for (const auto& resourceState : mFinalResourceState)
    {
        msGlobalResourceState[resourceState.first] = resourceState.second;
    }

    mFinalResourceState.clear();
}

void ResourceStateTracker::reset()
{
    mPendingResourceBarriers.clear();
    mResourceBarriers.clear();
    mFinalResourceState.clear();
}

void ResourceStateTracker::lock()
{
    msGlobalMutex.lock();
    msIsLocked = true;
}

void ResourceStateTracker::unlock()
{
    msIsLocked = false;
    msGlobalMutex.unlock();
}


void ResourceStateTracker::addGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
    if (resource != nullptr)
    {
        std::lock_guard<std::mutex> lock(msGlobalMutex);
        msGlobalResourceState[resource].setSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
    }
}

void ResourceStateTracker::removeGlobalResourceState(ID3D12Resource* resource)
{
    if (resource != nullptr)
    {
        std::lock_guard<std::mutex> lock(msGlobalMutex);
        msGlobalResourceState.erase(resource);
    }
}
