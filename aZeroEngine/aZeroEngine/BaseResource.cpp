#include "BaseResource.h"

BaseResource::BaseResource()
{
	state = D3D12_RESOURCE_STATE_COMMON;
}

BaseResource::~BaseResource()
{
	//resource->Release();
}

void BaseResource::Transition(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState)
{
	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, state, _newState));
	_gCmdList->ResourceBarrier(1, &barrier);
	state = _newState;
}
