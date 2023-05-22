#pragma once
#include "D3D12Include.h"
#include <typeinfo>

/** @brief Contains necessary information of a D3D12 descriptor handle.
* Contains an internal gpu handle, cpu handle and index into a descriptor heap.
*/
class DescriptorHandle
{
private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
	int m_heapIndex = -1;

public:
	/** Sets both GPU and CPU handles to NULL and the descriptor heap index to -1.
	*/
	DescriptorHandle()
		:m_heapIndex(-1)
	{
		m_cpuHandle.ptr = NULL;
		m_gpuHandle.ptr = NULL;
	}

	/** Copies the input data to the gpu handle, CPU handle, and heap index.
	@param cpuHandle CPU handle to the internal CPU handle.
	@param gpuHandle GPU handle to the internal CPU handle.
	@param heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle, int heapIndex)
		:m_cpuHandle(cpuHandle), m_gpuHandle(gpuHandle), m_heapIndex(heapIndex)
	{
	}

	/** Copies the input data to the CPU handle and heap index. Sets the GPU handle to NULL.
	@param _cpuHandle CPU handle to copy to the internal CPU handle.
	@param _heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, int heapIndex)
		:m_cpuHandle(cpuHandle), m_heapIndex(heapIndex)
	{
		m_gpuHandle.ptr = NULL;
	}

	~DescriptorHandle() = default;

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void setHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { m_cpuHandle = cpuHandle; }

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void setHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { m_gpuHandle = gpuHandle; }

	/** Copies the input data to the handle.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void setHandle(DescriptorHandle handle) { m_gpuHandle = handle.getGPUHandle(); m_cpuHandle = handle.getCPUHandle(); m_heapIndex = handle.getHeapIndex(); }

	/** Set internal heap index to the input value.
	@param _index Value to set the heap index to.
	@return void
	*/
	void setHeapIndex(int index) { m_heapIndex = index; }

	/** Returns a constant copy of the CPU handle.
	@return void
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle() const { return m_cpuHandle; }

	/** Returns a constant copy of the GPU handle.
	@return void
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const { return m_gpuHandle; }

	/** Returns a constant copy of the heap index.
	@return void
	*/
	int getHeapIndex() const { return m_heapIndex; }

	/** Returns a reference to the CPU handle.
	@return void
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE& getCPUHandleRef() { return m_cpuHandle; }

	/** Returns a reference to the GPU handle.
	@return void
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE& getGPUHandleRef() { return m_gpuHandle; }
};