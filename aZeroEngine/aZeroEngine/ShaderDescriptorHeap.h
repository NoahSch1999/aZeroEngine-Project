#pragma once
#include "DescriptorHandle.h"

/** @brief Class that encapsulates the logic for a shader visible descriptor heap.
*/
class ShaderDescriptorHeap
{
private:
	int maxDescriptors = -1;
	int descriptorSize = -1;
	ID3D12DescriptorHeap* heap;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	DescriptorHandle handle;

public:
	ShaderDescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	@param _name Optional D3D12 resource name for the internal ID3D12DescriptorHeap pointer.
	*/
	ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	@param _name Optional D3D12 resource name for the internal ID3D12DescriptorHeap pointer.
	@return void
	*/
	void Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, const std::wstring& _name = L"");
	~ShaderDescriptorHeap();

	/** @brief Returns a copy of a descriptor handle at the specified index.
	@param _index Index of the descriptor handle to return.
	@return DescriptorHandle
	*/
	DescriptorHandle GetNewDescriptorHandle(int _index);

	// OBSOLETE
	//// Used after all views have been created with the cpuhandles
	//// To use the resources gpu handles, take this gpu heap start, add hiddenheap start to gpu pntr, add resource hiddenheap index
	//// ~~~~~~ handle.gpuaddress += hiddenheap.offsetInShaderHeap + resource.handle.heapIndex ~~~~~~
	//void CopyFromHiddenHeap(ID3D12Device* _device, HiddenDescriptorHeap* _hiddenHeap);

	//// Updates the input descriptor handle with the correct gpu handle from the shader visible heap using the input hidden heap offset within the shader visible heap
	//D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleForHiddenResource(HiddenDescriptorHeap* _hiddenHeap, DescriptorHandle& _handle);
	//

	/** @brief Returns a copy of the max number of descriptors that the heap has.
	@return int
	*/
	int GetMaxDescriptors() { return maxDescriptors; }

	/** @brief Returns a copy of the descriptor size.
	@return int
	*/
	int GetDescriptorSize() { return descriptorSize; }

	/** @brief Returns a pointer to the internal heap.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetHeap() { return heap; }

	/** @brief Returns a copy of the heap type.
	@return D3D12_DESCRIPTOR_HEAP_TYPE
	*/
	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() { return type; }

	/** @brief Returns a reference to the handle of the heaps' start.
	@return DescriptorHandle&
	*/
	DescriptorHandle& GetDescriptorHandle() { return handle; }
};

