#pragma once
#include "D3D12Include.h"
#include <typeinfo>

/** @brief Contains necessary information of a D3D12 descriptor handle.
* Contains an internal gpu handle, cpu handle and index into a descriptor heap.
*/
class DescriptorHandle
{
private:
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	int heapIndex;

public:
	/** Sets both GPU and CPU handles to NULL and the descriptor heap index to -1.
	*/
	DescriptorHandle()
		:heapIndex(-1)
	{
		cpuHandle.ptr = NULL;
		gpuHandle.ptr = NULL;
	}

	/** Copies the input data to the gpu handle, CPU handle, and heap index.
	@param _cpuHandle CPU handle to the internal CPU handle.
	@param _gpuHandle GPU handle to the internal CPU handle.
	@param _heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& _gpuHandle, int _heapIndex)
		:cpuHandle(_cpuHandle), gpuHandle(_gpuHandle), heapIndex(_heapIndex)
	{
	}

	/** Copies the input data to the CPU handle and heap index. Sets the GPU handle to NULL.
	@param _cpuHandle CPU handle to copy to the internal CPU handle.
	@param _heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, int _heapIndex)
		:cpuHandle(_cpuHandle), heapIndex(_heapIndex)
	{
		gpuHandle.ptr = NULL;
	}

	~DescriptorHandle() = default;

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void SetHandle(D3D12_CPU_DESCRIPTOR_HANDLE _CPUHandle) { cpuHandle = _CPUHandle; }

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void SetHandle(D3D12_GPU_DESCRIPTOR_HANDLE _GPUHandle) { gpuHandle = _GPUHandle; }

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void SetHandle(DescriptorHandle _handle) { gpuHandle = _handle.GetGPUHandle(); cpuHandle = _handle.GetCPUHandle(); heapIndex = _handle.GetHeapIndex(); }

	/** Set internal heap index to the input value.
	@param _index Value to set the heap index to.
	@return void
	*/
	void SetHeapIndex(int _index) { heapIndex = _index; }

	/** Returns a constant copy of the CPU handle.
	@return void
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return cpuHandle; }

	/** Returns a constant copy of the GPU handle.
	@return void
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return gpuHandle; }

	/** Returns a constant copy of the heap index.
	@return void
	*/
	int GetHeapIndex() const { return heapIndex; }

	/** Returns a reference to the CPU handle.
	@return void
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandleRef() { return cpuHandle; }

	/** Returns a reference to the GPU handle.
	@return void
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandleRef() { return gpuHandle; }
};