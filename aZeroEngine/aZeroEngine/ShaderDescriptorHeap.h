#pragma once
#include "DescriptorHeap.h"

/** @brief A subclass of DescriptorHeap that encapsulates the logic for a shader visible DescriptorHeap.
*/
class ShaderDescriptorHeap : public DescriptorHeap
{
public:
	ShaderDescriptorHeap() = default;

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	*/
	ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors);

	virtual ~ShaderDescriptorHeap() {}

	/** @brief Initiates the necessary variables.
	@param _device Device to create the resources with.
	@param _type D3D12_DESCRIPTOR_HEAP_TYPE of the heap.
	@param _maxDescriptors Max number of descriptors that the heap can contain.
	@return void
	*/
	void Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors);
};

