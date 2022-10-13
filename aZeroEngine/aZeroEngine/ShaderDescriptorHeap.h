#pragma once
#include "DescriptorHeap.h"

class ShaderDescriptorHeap : public DescriptorHeap
{
public:
	ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");
	~ShaderDescriptorHeap();

	DescriptorHandle GetNewDescriptorHandle(int _numHandles);
	DescriptorHandle GetExistingDescriptorHandle(int _heapIndex);
};

