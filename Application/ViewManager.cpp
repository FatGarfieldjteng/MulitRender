#include "framework.h"
#include "ViewManager.h"
#include "Device.h"
#include "RootSignature.h"
#include "CommandList.h"

#include <stdexcept>

ViewManager::ViewManager(std::shared_ptr<Device> device, 
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	uint32_t numDescriptorsPerHeap)
	: mDevice(device)
	, mDescriptorHeapType(type)
	, mNumDescriptorsPerHeap(numDescriptorsPerHeap)
{
	mDescriptorHandleIncrementSize = mDevice->getDescriptorHandleIncrementSize(type);

	// allocate empty space for staging CPU visible descriptors.
	mCPUDescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(mNumDescriptorsPerHeap);
}

ViewManager::~ViewManager()
{

}

void ViewManager::parseRootSignature(const RootSignature& rootSignature)
{
	
	mStaleDescriptorTableBitMask = 0;

	const auto& rootSignatureDesc = rootSignature.desc();

	mDescriptorTableBitMask = rootSignature.descriptorTableBitMask(mDescriptorHeapType);
	uint32_t descriptorTableBitMask = mDescriptorTableBitMask;
	uint32_t currentOffset = 0;
	DWORD rootIndex;
	while (_BitScanForward(&rootIndex, descriptorTableBitMask) && rootIndex < rootSignatureDesc.NumParameters)
	{
		uint32_t numDescriptors = rootSignature.numDescriptors(rootIndex);

		CPUDescriptorTableCacheEntry& cpuDscriptorTableCacheEntry = mCPUDescriptorTableCache[rootIndex];

		cpuDscriptorTableCacheEntry.NumCPUDescriptors = numDescriptors;
		cpuDscriptorTableCacheEntry.BaseCPUDescriptor = mCPUDescriptorHandleCache.get() + currentOffset;

		currentOffset += numDescriptors;

		// clear the found bit
		descriptorTableBitMask ^= (1 << rootIndex);
	}


	// validation
	assert(currentOffset <= mNumDescriptorsPerHeap);
}


void ViewManager::stageCPUDescriptors(uint32_t rootParameterIndex,
	uint32_t offset,
	uint32_t numCPUDescriptors,
	const D3D12_CPU_DESCRIPTOR_HANDLE srcBaseCPUDescriptor)
{
	// validation
	if (numCPUDescriptors > mNumDescriptorsPerHeap || rootParameterIndex >= MaxDescriptorTables)
	{
		throw std::bad_alloc();
	}

	CPUDescriptorTableCacheEntry& descriptorTableCacheEntry = mCPUDescriptorTableCache[rootParameterIndex];

	// Check that the number of descriptors to copy does not exceed the number
	// of descriptors expected in the descriptor table.
	if ((offset + numCPUDescriptors) > descriptorTableCacheEntry.NumCPUDescriptors)
	{
		throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table.");
	}


	D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCacheEntry.BaseCPUDescriptor + offset);
	for (uint32_t i = 0; i < numCPUDescriptors; ++i)
	{
		dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcBaseCPUDescriptor, i, mDescriptorHandleIncrementSize);
	}

	// mark as dirty
	mStaleDescriptorTableBitMask |= (1 << rootParameterIndex);
}


uint32_t ViewManager::computeStaleDescriptorCount() const
{
	uint32_t numStaleDescriptors = 0;
	DWORD i;
	DWORD staleDescriptorsBitMask = mStaleDescriptorTableBitMask;

	while (_BitScanForward(&i, staleDescriptorsBitMask))
	{
		numStaleDescriptors += mCPUDescriptorTableCache[i].NumCPUDescriptors;
		staleDescriptorsBitMask ^= (1 << i);
	}

	return numStaleDescriptors;
}

ComPtr<ID3D12DescriptorHeap> ViewManager::acquireDescriptorHeap()
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	if (!mAvailableDescriptorHeaps.empty())
	{
		descriptorHeap = mAvailableDescriptorHeaps.front();
		mAvailableDescriptorHeaps.pop();
	}
	else
	{
		descriptorHeap = createDescriptorHeap();
		mDescriptorHeapPool.push(descriptorHeap);
	}

	return descriptorHeap;
}

ComPtr<ID3D12DescriptorHeap> ViewManager::createDescriptorHeap()
{
	
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = mDescriptorHeapType;
	descriptorHeapDesc.NumDescriptors = mNumDescriptorsPerHeap;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	mDevice->createDescriptorHeap(&descriptorHeapDesc);

	return descriptorHeap;
}

void ViewManager::commitDescriptorTables(CommandList& commandList,
	std::function<void(ID3D12GraphicsCommandList*, 
		UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
{
	// Compute the number of descriptors that need to be copied 
	uint32_t numDescriptorsToCommit = computeStaleDescriptorCount();

	if (numDescriptorsToCommit > 0)
	{
		auto dxcommandList = commandList.commandList().Get();
		assert(dxcommandList != nullptr);

		// if no descriptor heap can meet the requirement, acquire or create one
		if (!mCurrentDescriptorHeap || mNumFreeHandles < numDescriptorsToCommit)
		{
			mCurrentDescriptorHeap = acquireDescriptorHeap();
			mCurrentCPUDescriptorHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			mCurrentGPUDescriptorHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			mNumFreeHandles = mNumDescriptorsPerHeap;

			commandList.descriptorHeap(mDescriptorHeapType, mCurrentDescriptorHeap.Get());

			// When updating the descriptor heap on the command list, all descriptor
			// tables must be (re)recopied to the new descriptor heap (not just
			// the stale descriptor tables).
			mStaleDescriptorTableBitMask = mDescriptorTableBitMask;
		}

		DWORD rootIndex;
		// Scan from LSB to MSB for a bit set in staleDescriptorsBitMask
		while (_BitScanForward(&rootIndex, mStaleDescriptorTableBitMask))
		{
			UINT numSrcDescriptors = mCPUDescriptorTableCache[rootIndex].NumCPUDescriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorHandles = mCPUDescriptorTableCache[rootIndex].BaseCPUDescriptor;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[] =
			{
				mCurrentCPUDescriptorHandle
			};
			UINT pDestDescriptorRangeSizes[] =
			{
				numSrcDescriptors
			};

			// copy CPU handles to GPU visible heap
			mDevice->copyDescriptors(1, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
				numSrcDescriptors, pSrcDescriptorHandles, nullptr, mDescriptorHeapType);

			// Set the descriptors on the command list using the passed-in setter function.
			setFunc(dxcommandList, rootIndex, mCurrentGPUDescriptorHandle);

			// Offset current CPU and GPU descriptor handles.
			mCurrentCPUDescriptorHandle.Offset(numSrcDescriptors, mDescriptorHandleIncrementSize);
			mCurrentGPUDescriptorHandle.Offset(numSrcDescriptors, mDescriptorHandleIncrementSize);
			mNumFreeHandles -= numSrcDescriptors;

			// clear the bit to avoid _BitScanForward scan the same bit
			mStaleDescriptorTableBitMask ^= (1 << rootIndex);
		}
	}
}

void ViewManager::commitStagedDescriptorsForDraw(CommandList& commandList)
{
	commitDescriptorTables(commandList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
}

void ViewManager::commitStagedDescriptorsForDispatch(CommandList& commandList)
{
	commitDescriptorTables(commandList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
}

D3D12_GPU_DESCRIPTOR_HANDLE ViewManager::copyDescriptor(CommandList& comandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
{
	if (!mCurrentDescriptorHeap || mNumFreeHandles < 1)
	{
		mCurrentDescriptorHeap = acquireDescriptorHeap();
		mCurrentCPUDescriptorHandle = mCurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		mCurrentGPUDescriptorHandle = mCurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		mNumFreeHandles = mNumDescriptorsPerHeap;

		comandList.descriptorHeap(mDescriptorHeapType, mCurrentDescriptorHeap.Get());

		mStaleDescriptorTableBitMask = mDescriptorTableBitMask;
	}


	D3D12_GPU_DESCRIPTOR_HANDLE hGPU = mCurrentGPUDescriptorHandle;
	mDevice->copyDescriptorsSimple(1, mCurrentCPUDescriptorHandle, cpuDescriptor, mDescriptorHeapType);

	mCurrentCPUDescriptorHandle.Offset(1, mDescriptorHandleIncrementSize);
	mCurrentGPUDescriptorHandle.Offset(1, mDescriptorHandleIncrementSize);
	mNumFreeHandles -= 1;

	return hGPU;
}

void ViewManager::reset()
{
	mAvailableDescriptorHeaps = mDescriptorHeapPool;
	mCurrentDescriptorHeap.Reset();
	mCurrentCPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	mCurrentGPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
	mNumFreeHandles = 0;
	mDescriptorTableBitMask = 0;
	mStaleDescriptorTableBitMask = 0;

	// Reset the table cache
	for (int i = 0; i < MaxDescriptorTables; ++i)
	{
		mCPUDescriptorTableCache[i].reset();
	}
}