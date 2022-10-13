#include "ShaderDescriptorHeap.h"

ShaderDescriptorHeap::ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name)
	:DescriptorHeap(_device, _type, _maxDescriptors, true, _name)
{
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

	D3D12_CPU_DESCRIPTOR_HANDLE newCPUHandle = cpuAddress;
	newCPUHandle.ptr += newHandleIndex * descriptorSize;
	D3D12_GPU_DESCRIPTOR_HANDLE newGPUHandle = gpuAddress;
	newGPUHandle.ptr += newHandleIndex * descriptorSize;

	DescriptorHandle handle(newCPUHandle, newGPUHandle, newHandleIndex);

	return handle;
}

DescriptorHandle ShaderDescriptorHeap::GetExistingDescriptorHandle(int _heapIndex)
{
	if (_heapIndex > maxDescriptors)
		throw;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = cpuAddress;
	cpuHandle.ptr += _heapIndex * descriptorSize;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = gpuAddress;
	gpuHandle.ptr += _heapIndex * descriptorSize;

	DescriptorHandle handle(cpuHandle, gpuHandle, _heapIndex);
	
	return handle;
}
