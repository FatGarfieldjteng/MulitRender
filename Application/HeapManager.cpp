#include "framework.h"
#include "HeapManager.h"

HeapManager::HeapManager()
{

}

HeapManager::~HeapManager()
{

}

void HeapManager::addHeap(const std::string& ID, ComPtr<ID3D12DescriptorHeap> heap)
{
	mIDToHeap[ID] = heap;
}

ComPtr<ID3D12DescriptorHeap> HeapManager::getHeap(const std::string& ID)
{
	std::map<std::string, ComPtr<ID3D12DescriptorHeap> > ::iterator it = mIDToHeap.find(ID);

	if (it != mIDToHeap.end())
	{
		return  it->second;
	}
	else
	{
		return nullptr;
	}
}