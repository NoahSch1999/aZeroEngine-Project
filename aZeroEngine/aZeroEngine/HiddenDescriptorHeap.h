#pragma once
#include "DescriptorHeap.h"

class HiddenDescriptorHeap : DescriptorHeap
{
public:
	HiddenDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");
	~HiddenDescriptorHeap();

	DescriptorHandle GetNewDescriptorHandle(int _numHandles);
};

