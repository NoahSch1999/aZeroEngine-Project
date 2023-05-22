#pragma once
#include "DescriptorHandle.h"
#include "UniqueIntList.h"

/** @brief Class that encapsulates the logic for a shader visible descriptor heap.
*/
class DescriptorHeap
{
protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	DescriptorHandle m_handle;

	UniqueIntList m_descriptorList;

	int m_maxDescriptors = -1;
	int m_descriptorSize = -1;
	bool m_isShaderVisible = false;

public:
	DescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param device Device to create the resources with.
	@param type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param maxDescriptors Max number of descriptors that the heap can contain.
	@param flags D3D12_DESCRIPTOR_HEAP_FLAGS for the ID3D12DescriptorHeap.
	*/
	DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

	~DescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param device Device to create the resources with.
	@param type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param maxDescriptors Max number of descriptors that the heap can contain.
	@param flags D3D12_DESCRIPTOR_HEAP_FLAGS for the ID3D12DescriptorHeap.
	@return void
	*/
	void init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags);

	/** @brief Returns a pointer to the internal heap.
	@return ID3D12DescriptorHeap*
	*/
	ID3D12DescriptorHeap* getHeap() const { return m_heap.Get(); }

	/** @brief Returns the heap type.
	@return D3D12_DESCRIPTOR_HEAP_TYPE
	*/
	D3D12_DESCRIPTOR_HEAP_TYPE getHeapType() const { return m_type; }

	/** @brief Returns the DescriptorHandle of the heaps' start.
	@return DescriptorHandle
	*/
	DescriptorHandle getStartDescriptorHandle() const { return m_handle; }

	/** @brief Returns the descriptor size.
	@return int
	*/
	int getDescriptorSize() const { return m_descriptorSize; }

	/** @brief Returns the max number of DescriptorHandle objects that the heap has.
	@return int
	*/
	int getMaxDescriptors() const { return m_maxDescriptors; }

	/** Returns a new unused DescriptorHandle of the heap type specified during DescriptorHeap initiation.
	@return DescriptorHandle
	*/
	DescriptorHandle getDescriptorHandle();

	/**Returns a std::vector of new unused DescriptorHandle objects of the heap type specified during DescriptorHeap initiation.
	@return std::vector<DescriptorHandle>
	*/
	std::vector<DescriptorHandle> getDescriptorHandles(int numHandles);

	/** Frees up the input DescriptorHandle for a resource which allows it to be reused by calling DescriptorHeap::GetNewDescriptorHandle().
	@param _handle The DescriptorHandle to free. The reference is modified and will contain a heap index of -1 to indicate it being freed.
	@return void
	*/
	void freeDescriptorHandle(DescriptorHandle& handle);

	int getNumberOfUsedDescriptors() const { return m_descriptorList.getNumCurrentlyLent(); }
};