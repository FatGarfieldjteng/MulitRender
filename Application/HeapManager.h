#pragma once

#include <d3d12.h>
#include <memory>
#include <map>
#include <string>

class HeapManager
{
public:

    HeapManager();

    ~HeapManager();

public:
    void addHeap(const std::string& ID, ComPtr<ID3D12DescriptorHeap> heap);
    ComPtr<ID3D12DescriptorHeap> getHeap(const std::string& ID);

private:
    std::map<std::string, ComPtr<ID3D12DescriptorHeap> > mIDToHeap;
};
