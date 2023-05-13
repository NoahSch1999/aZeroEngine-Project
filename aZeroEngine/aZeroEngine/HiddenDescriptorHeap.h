#pragma once
#include "DescriptorHeap.h"

/** @brief A subclass of DescriptorHeap that encapsulates the logic for a non-shader visible DescriptorHeap.
*/
class HiddenDescriptorHeap : public DescriptorHeap
{
public:
	HiddenDescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param device Device to create the resources with.
	@param type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param maxDescriptors Max number of descriptors that the heap can contain.
	*/
	HiddenDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors);

	virtual ~HiddenDescriptorHeap() {}

	/** @brief Initiates the necessary variables.
	@param device Device to create the resources with.
	@param type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param maxDescriptors Max number of descriptors that the heap can contain.
	@return void
	*/
	void init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors);
};