#pragma once
#include "DescriptorHandle.h"

class BaseResource
{
private:
protected:
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
public:
	BaseResource();
	virtual ~BaseResource();
	ID3D12Resource* resource;
	D3D12_RESOURCE_STATES state;
	DescriptorHandle handle;
	void Transition(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState);
	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return gpuAddress; }
	void SetGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS _gpuAddress) { gpuAddress = _gpuAddress; }
};

