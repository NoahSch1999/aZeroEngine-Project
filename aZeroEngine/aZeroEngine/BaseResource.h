#pragma once
#include "DescriptorHandle.h"
#include "HelperFunctions.h"

/** @brief The base class for all resources.
*/
class BaseResource
{
protected:
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = (UINT64)0;
	ID3D12Resource* mainResource = nullptr;
	D3D12_RESOURCE_STATES mainResourceState = D3D12_RESOURCE_STATE_COMMON;
	ID3D12Resource* intermediateResource = nullptr;
	D3D12_RESOURCE_STATES intermediateResourceState = D3D12_RESOURCE_STATE_COMMON;
	DescriptorHandle handle;
	bool isTrippleBuffered = false;
	int sizePerSubresource = -1;
	void* mappedBuffer = nullptr;

public:
	bool dirty = false;
	constexpr int GetSizePerSubresource() const { return sizePerSubresource; }
	constexpr void* GetMappedBuffer() const { return mappedBuffer; }

	BaseResource() = default;
	virtual ~BaseResource(){}

	/**Returns whether the resource is tripple buffered or not.
	@return bool
	*/
	constexpr bool IsTrippleBuffered() { return isTrippleBuffered; }

	/**Return the GPU virtual address.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	constexpr virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return gpuAddress; }

	/**Sets the GPU virtual address.
	@param _gpuAddress The new virtual GPU address.
	@return void
	*/
	void SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS _gpuAddress) { gpuAddress = _gpuAddress; }

	/**Returns a pointer reference to the internal main ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	constexpr ID3D12Resource*& GetMainResource() { return mainResource; }

	/**Returns a pointer reference to the internal intermediate ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	constexpr ID3D12Resource*& GetIntermediateResource() { return intermediateResource; }

	/**Returns the current resource state.
	@return D3D12_RESOURCE_STATES
	*/
	constexpr D3D12_RESOURCE_STATES GetMainState() const { return mainResourceState; }

	/**Returns the current resource state.
	@return D3D12_RESOURCE_STATES
	*/
	constexpr D3D12_RESOURCE_STATES GetIntermediateState() const { return intermediateResourceState; }

	/**Sets the current main resource state.
	@param _newState New state.
	@return void
	*/
	inline void SetMainState(D3D12_RESOURCE_STATES _newState) { mainResourceState = _newState; }

	/**Sets the current intermediate resource state.
	@param _newState New state.
	@return void
	*/
	inline void SetIntermediateState(D3D12_RESOURCE_STATES _newState) { intermediateResourceState = _newState; }

	/**Returns a pointer reference to the internal ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	constexpr DescriptorHandle& GetHandle() { return handle; }

};