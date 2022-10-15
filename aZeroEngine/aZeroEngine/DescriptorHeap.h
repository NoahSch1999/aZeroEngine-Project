#pragma once
#include "DescriptorHandle.h"

// Descriptor tells Direct3D how the resource will be used (i.e. what stage of the pipeline)
// Descriptor tells Direct3D what state the resource is in (how it can be used, i.e. read, write etc.)

class DescriptorHeap
{
public:
	DescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, bool _shaderVisable = true, const std::wstring& _name = L"");
	virtual ~DescriptorHeap();

	int maxDescriptors;
	int numDescriptors;
	int descriptorSize;
	int currentIndex;
	ID3D12DescriptorHeap* heap;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuAddress;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuAddress;
	ID3D12DescriptorHeap* GetHeap() { return heap; }
};

