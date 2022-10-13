#pragma once
#include "D3D12Include.h"

class DescriptorHandle
{
public:
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	int heapIndex;

	DescriptorHandle()
	{
		cpuHandle.ptr = NULL;
		gpuHandle.ptr = NULL;
		heapIndex = 0;
	}

	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& _gpuHandle, int _heapIndex)
	{
		cpuHandle = _cpuHandle;
		gpuHandle = _gpuHandle;
		heapIndex = _heapIndex;
	}

	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, int _heapIndex)
	{
		cpuHandle = _cpuHandle;
		gpuHandle.ptr = NULL;
		heapIndex = _heapIndex;
	}

	~DescriptorHandle()
	{
	}


};