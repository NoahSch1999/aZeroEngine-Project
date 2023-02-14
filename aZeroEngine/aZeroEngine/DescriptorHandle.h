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
	{
		cpuHandle.ptr = NULL;
		gpuHandle.ptr = NULL;
		heapIndex = -1;
	}

	/** Copies the input data to the gpu handle, CPU handle, and heap index.
	@param _cpuHandle CPU handle to the internal CPU handle.
	@param _gpuHandle GPU handle to the internal CPU handle.
	@param _heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& _gpuHandle, int _heapIndex)
	{
		cpuHandle = _cpuHandle;
		gpuHandle = _gpuHandle;
		heapIndex = _heapIndex;
	}

	/** Copies the input data to the CPU handle and heap index. Sets the GPU handle to NULL.
	@param _cpuHandle CPU handle to copy to the internal CPU handle.
	@param _heapIndex Index to set the internal heap index to.
	*/
	DescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& _cpuHandle, int _heapIndex)
	{
		cpuHandle = _cpuHandle;
		gpuHandle.ptr = NULL;
		heapIndex = _heapIndex;
	}

	~DescriptorHandle() = default;

	/** Copies the input data to the handle. 
	* If T is D3D12_CPU_DESCRIPTOR_HANDLE, the CPU handle will be set. 
	* If T is D3D12_GPU_DESCRIPTOR_HANDLE, the GPU handle will be set.
	* If T is DescriptorHandle, the DescriptorHandle will be copied to this object
	@param _handle Handle to copy to the internal handle.
	*/
	template<typename T>
	void SetHandle(const T& _handle);

	/** Set internal heap index to the input value.
	@param _index Value to set the heap index to.
	*/
	void SetHeapIndex(int _index) { heapIndex = _index; }

	/** Returns a constant copy of the CPU handle.
	*/
	const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle()const { return cpuHandle; }

	/** Returns a constant copy of the GPU handle.
	*/
	const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle()const { return gpuHandle; }

	/** Returns a constant copy of the heap index.
	*/
	const int GetHeapIndex()const { return heapIndex; }

	/** Returns a reference to the CPU handle.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandleRef() { return cpuHandle; }

	/** Returns a reference to the GPU handle.
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandleRef() { return gpuHandle; }
};

template<typename T>
inline void DescriptorHandle::SetHandle(const T& _handle)
{
	if constexpr (std::is_same_v<T, D3D12_CPU_DESCRIPTOR_HANDLE>)
	{
		cpuHandle = _handle;
	}
	else if constexpr (std::is_same_v<T, D3D12_GPU_DESCRIPTOR_HANDLE>)
	{
		gpuHandle = _handle;
	}
	else if constexpr (std::is_same_v<T, DescriptorHandle>)
	{
		cpuHandle = _handle.GetCPUHandle();
		gpuHandle = _handle.GetGPUHandle();
		heapIndex = _handle.GetHeapIndex();
	}
	else
	{
		static_assert(std::is_same_v<T, D3D12_CPU_DESCRIPTOR_HANDLE> || std::is_same_v<T, D3D12_GPU_DESCRIPTOR_HANDLE> || std::is_same_v<T, DescriptorHandle>, "T is an invalid input.");
		//using FailureType = typename std::enable_if<false, T>::type; // SFINAE
	}
}
