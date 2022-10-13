#pragma once
#include "DescriptorHeap.h"

class BaseResource
{
public:
	BaseResource();
	virtual ~BaseResource();
	ID3D12Resource* resource;
	D3D12_RESOURCE_STATES state;
	DescriptorHandle handle;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	void Transition(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState);
};

