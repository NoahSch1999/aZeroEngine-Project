#include "HiddenDescriptorHeap.h"

HiddenDescriptorHeap::HiddenDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors)
{
	HiddenDescriptorHeap::init(device, type, maxDescriptors);
}

void HiddenDescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors)
{
	DescriptorHeap::init(device, type, maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
}
