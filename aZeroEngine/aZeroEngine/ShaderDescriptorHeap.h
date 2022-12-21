#pragma once
#include "HiddenDescriptorHeap.h"
#include "BaseResource.h"
#include "FreeList.h"

class ShaderDescriptorHeap
{
public:
	int maxDescriptors = -1;
	int descriptorSize = -1;
	ID3D12DescriptorHeap* heap;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	DescriptorHandle handle;

	ShaderDescriptorHeap() = default;
	ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");
	void Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");
	~ShaderDescriptorHeap();

	DescriptorHandle GetNewDescriptorHandle(int _index);

	// OBSOLETE
	//// Used after all views have been created with the cpuhandles
	//// To use the resources gpu handles, take this gpu heap start, add hiddenheap start to gpu pntr, add resource hiddenheap index
	//// ~~~~~~ handle.gpuaddress += hiddenheap.offsetInShaderHeap + resource.handle.heapIndex ~~~~~~
	//void CopyFromHiddenHeap(ID3D12Device* _device, HiddenDescriptorHeap* _hiddenHeap);

	//// Updates the input descriptor handle with the correct gpu handle from the shader visible heap using the input hidden heap offset within the shader visible heap
	//D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleForHiddenResource(HiddenDescriptorHeap* _hiddenHeap, DescriptorHandle& _handle);
	//
	
	void Reset();
};

