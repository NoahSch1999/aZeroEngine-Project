#pragma once
#include "DescriptorHandle.h"
#include "CommandList.h"
#include "HelperFunctions.h"

/** @brief Base class for all buffer resources within aZero Engine.
*/
class BufferResource
{
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> gpuOnlyResource = nullptr;
	D3D12_RESOURCE_STATES gpuOnlyResourceState = D3D12_RESOURCE_STATE_COMMON;

	Microsoft::WRL::ComPtr<ID3D12Resource> uploadResource = nullptr;
	D3D12_RESOURCE_STATES uploadResourceState = D3D12_RESOURCE_STATE_COMMON;

	DescriptorHandle handle;

	bool isTrippleBuffered = false;
	int sizePerSubresource = -1;
	void* mappedBuffer = nullptr;
	bool isDynamic = false;

	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = (UINT64)0;

public:
	BufferResource() = default;
	virtual ~BufferResource(){}

	/* Initializes the UniqueIntList object.
	@param _device ID3D12Device for resource creations.
	@param _copyList CommandList for recording upload copy commands on.
	@param _width Width of the _data argument in bytes.
	@param _trippleBuffered Whether or not the BufferResource is tripplebuffered or single buffered.
	@param _data Initial data to for the resource. If the argument is nullptr the buffer will be filled with 0.
	@param _dynamic Whether or not the resource is dynamic. _trippleBuffered will be ignored if this is false.
	@return void
	*/
	void Init(ID3D12Device* _device, CommandList& _copyList, int _width, bool _trippleBuffered, void* _data, bool _dynamic = true);

	/* Returns the size of the GPU-only resource in bytes. Same as the _width parameter for BufferResource::Init().
	@return int Size of GPU-only resource in bytes.
	*/
	int GetSizePerSubresource() const { return sizePerSubresource; }

	/* Returns a pointer to the upload buffer mapped memory. Nullptr if the BufferResource isn't dynamic.
	@return void*
	*/
	void* GetMappedBuffer() const { return mappedBuffer; }

	/* Returns whether or not the BufferResource is tripplebuffered.
	@return bool
	*/
	bool IsTrippleBuffered() { return isTrippleBuffered; }

	/* Returns the D3D12_GPU_VIRTUAL_ADDRESS for the GPU-only buffer.
	@return D3D12_GPU_VIRTUAL_ADDRESS
	*/
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return gpuAddress; }

	/*
	@param
	@return
	*/
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetGPUOnlyResource() { return gpuOnlyResource; }

	/*
	@param
	@return
	*/
	D3D12_RESOURCE_STATES GetGPUOnlyResourceState() const { return gpuOnlyResourceState; }

	/*
	@param
	@return
	*/
	void SetGPUOnlyResourceState(D3D12_RESOURCE_STATES _newState) { gpuOnlyResourceState = _newState; }

	/*
	@param
	@return
	*/
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetUploadResource() { return uploadResource; }

	/*
	@param
	@return
	*/
	D3D12_RESOURCE_STATES GetUploadResourceState() const { return uploadResourceState; }

	/*
	@param
	@return
	*/
	void SetUploadResourceState(D3D12_RESOURCE_STATES _newState) { uploadResourceState = _newState; }

	/*
	@param
	@return
	*/
	bool IsDynamic() { return isDynamic; }

	/*
	@param
	@return
	*/
	DescriptorHandle& GetHandle() { return handle; }
};