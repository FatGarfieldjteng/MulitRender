#pragma once

class ViewManager
{
public:

    ViewManager(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 1024);

    ~ViewManager();
};
