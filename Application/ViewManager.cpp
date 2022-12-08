#include "framework.h"
#include "ViewManager.h"
#include "Device.h"
#include "RootSignature.h"

#include <stdexcept>

ViewManager::ViewManager(std::shared_ptr<Device> device, 
	D3D12_DESCRIPTOR_HEAP_TYPE type,
	uint32_t numDescriptorsPerHeap)
	: mDescriptorHeapType(type)
	, mNumDescriptorsPerHeap(numDescriptorsPerHeap)
{
	mDescriptorHandleIncrementSize = device->getDescriptorHandleIncrementSize(type);

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
	mDevice->createDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

	return descriptorHeap;
}