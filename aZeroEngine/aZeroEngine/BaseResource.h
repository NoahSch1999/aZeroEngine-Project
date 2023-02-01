#pragma once
#include "DescriptorHandle.h"

/** @brief The base class for all resources.
*/
class BaseResource
{
protected:
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	ID3D12Resource* resource;
	D3D12_RESOURCE_STATES state;
	DescriptorHandle handle;
public:
	BaseResource();
	virtual ~BaseResource();

	/**Transitions the resource from one state to another.
	@param _gCmdList Pointer to a ID3D12GraphicsCommandList to register the transitioning to.
	@param _newState The state to transition to.
	@return void
	*/
	void Transition(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState);

	/**Return the GPU virtual address.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpuAddress; }

	/**Sets the GPU virtual address.
	@param _gpuAddress The new virtual GPU address.
	@return void
	*/
	void SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS _gpuAddress) { gpuAddress = _gpuAddress; }

	/**Returns a pointer reference to the internal ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	ID3D12Resource*& GetResource() { return resource; }

	/**Returns the current resource state.
	@return D3D12_RESOURCE_STATES
	*/
	D3D12_RESOURCE_STATES GetState() { return state; }

	/**Sets the current resource state.
	@param _newState New state.
	@return void
	*/
	void SetState(D3D12_RESOURCE_STATES _newState) { state = _newState; }

	/**Returns a pointer reference to the internal ID3D12Resource object.
	@return ID3D12Resource*&
	*/
	DescriptorHandle& GetHandle() { return handle; }

};

