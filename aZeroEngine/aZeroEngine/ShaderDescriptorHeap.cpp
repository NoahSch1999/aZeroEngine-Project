#include "ShaderDescriptorHeap.h"

ShaderDescriptorHeap::ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.NumDescriptors = _maxDescriptors;
	desc.Type = _type;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0; // Note - Remove?

	HRESULT hr = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
	if (FAILED(hr))
		throw;

	handle.cpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
	handle.gpuHandle = heap->GetGPUDescriptorHandleForHeapStart();
	descriptorSize = _device->GetDescriptorHandleIncrementSize(_type);

	type = _type;
	maxDescriptors = _maxDescriptors;
	numDescriptors = 0;
	currentIndex = 0;

	heap->SetName(_name.c_str());
}

ShaderDescriptorHeap::~ShaderDescriptorHeap()
{
}

DescriptorHandle ShaderDescriptorHeap::GetNewDescriptorHandle(int _numHandles)
{
	int newHandleIndex = 0;
	int handleBlockEnd = currentIndex + _numHandles;
	if (handleBlockEnd <= maxDescriptors)
	{
		newHandleIndex = currentIndex;
		currentIndex = handleBlockEnd;
	}
	else
	{
		throw;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE newCPUHandle = handle.cpuHandle;
	newCPUHandle.ptr += newHandleIndex * descriptorSize;
	D3D12_GPU_DESCRIPTOR_HANDLE newGPUHandle = handle.gpuHandle;
	newGPUHandle.ptr += newHandleIndex * descriptorSize;

	DescriptorHandle handle(newCPUHandle, newGPUHandle, newHandleIndex);

	return handle;
}


void ShaderDescriptorHeap::CopyFromHiddenHeap(ID3D12Device* _device, HiddenDescriptorHeap* _hiddenHeap)
{
	if (type != _hiddenHeap->type)
		throw;

	int totalDescriptorsUsed = _hiddenHeap->totalDescriptors - (_hiddenHeap->freeSlots.size() * _hiddenHeap->slotSize);
	D3D12_CPU_DESCRIPTOR_HANDLE destinationStart = handle.cpuHandle;
	destinationStart.ptr += currentIndex * descriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE sourceStart = _hiddenHeap->handle.cpuHandle;

	_device->CopyDescriptorsSimple(totalDescriptorsUsed, destinationStart, sourceStart, type);
	
	_hiddenHeap->offsetInShaderHeap = currentIndex;
	currentIndex += totalDescriptorsUsed;
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderDescriptorHeap::GetGPUHandleForHiddenResource(HiddenDescriptorHeap* _hiddenHeap, DescriptorHandle& _handle)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleToReturn = handle.gpuHandle;
	handleToReturn.ptr += (_hiddenHeap->offsetInShaderHeap + _handle.heapIndex) * descriptorSize;
	_handle.gpuHandle = handleToReturn;
	return handleToReturn;
}