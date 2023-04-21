#pragma once
#include "DescriptorHandle.h"
#include "UniqueIntList.h"

/** @brief Class that encapsulates the logic for a shader visible descriptor heap.
*/
class DescriptorHeap
{
protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap;
	D3D12_DESCRIPTOR_HEAP_TYPE type;
	DescriptorHandle handle;

	UniqueIntList descriptorList;

	int maxDescriptors = -1;
	int descriptorSize = -1;
	bool isShaderVisible = false;

public:
	DescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	@param _flags D3D12_DESCRIPTOR_HEAP_FLAGS for the ID3D12DescriptorHeap.
	*/
	DescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags);

	~DescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	@param _flags D3D12_DESCRIPTOR_HEAP_FLAGS for the ID3D12DescriptorHeap.
	@return void
	*/
	void Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags);

	/** @brief Returns a pointer to the internal heap.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* GetHeap() const { return heap.Get(); }

	/** @brief Returns the heap type.
	@return D3D12_DESCRIPTOR_HEAP_TYPE
	*/
	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return type; }

	/** @brief Returns the DescriptorHandle of the heaps' start.
	@return DescriptorHandle
	*/
	DescriptorHandle GetStartDescriptorHandle() const { return handle; }

	/** @brief Returns the descriptor size.
	@return int
	*/
	int GetDescriptorSize() const { return descriptorSize; }

	/** @brief Returns the max number of DescriptorHandle objects that the heap has.
	@return int
	*/
	int GetMaxDescriptors() const { return maxDescriptors; }

	/** Returns a new unused DescriptorHandle of the heap type specified during DescriptorHeap initiation.
	@return DescriptorHandle
	*/
	DescriptorHandle GetDescriptorHandle();

	/**Returns a std::vector of new unused DescriptorHandle objects of the heap type specified during DescriptorHeap initiation.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> GetDescriptorHandles(int _numHandles);

	/** Frees up the input DescriptorHandle for a resource which allows it to be reused by calling DescriptorHeap::GetNewDescriptorHandle().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void FreeDescriptorHandle(DescriptorHandle& _handle);
};