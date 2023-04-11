#include "HiddenDescriptorHeap.h"

HiddenDescriptorHeap::HiddenDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors)
{
	HiddenDescriptorHeap::Init(_device, _type, _maxDescriptors);
}

void HiddenDescriptorHeap::Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors)
{
	DescriptorHeap::Init(_device, _type, _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
}
