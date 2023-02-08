#include "BaseResource.h"

BaseResource::~BaseResource()
{
	/*if (isInitiated)
	{
		mainResource->Release();

		if (intermediateResource == nullptr)
		{
			intermediateResource->Release();
		}
	}*/
}

void BaseResource::ReleaseIntermediate()
{
	if (intermediateResource != nullptr)
		intermediateResource->Release();
}

void BaseResource::TransitionMain(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState)
{
	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(mainResource, mainResourceState, _newState));
	_gCmdList->ResourceBarrier(1, &barrier);
	mainResourceState = _newState;
}

void BaseResource::TransitionIntermediate(ID3D12GraphicsCommandList* _gCmdList, D3D12_RESOURCE_STATES _newState)
{
	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(intermediateResource, intermediateResourceState, _newState));
	_gCmdList->ResourceBarrier(1, &barrier);
	intermediateResourceState = _newState;
}

void BaseResource::Update(const void* _data, int _numBytes)
{
	if (_numBytes > sizePerSubresource)
		throw;

	memcpy((char*)mappedBuffer, _data, _numBytes);
}

void BaseResource::Update(CommandList* _cmdList, const void* _data, int _numBytes, int _frameIndex, int _offset)
{
	if (_numBytes > sizePerSubresource || _offset >= _numBytes)
		throw;

	memcpy((char*)mappedBuffer + sizePerSubresource * _frameIndex + _offset, _data, _numBytes);

	if (mainResourceState != D3D12_RESOURCE_STATE_COPY_DEST)
		TransitionMain(_cmdList->graphic, D3D12_RESOURCE_STATE_COPY_DEST);

	TransitionIntermediate(_cmdList->graphic, D3D12_RESOURCE_STATE_COPY_SOURCE);

	_cmdList->graphic->CopyBufferRegion(mainResource, 0, intermediateResource, sizePerSubresource * _frameIndex + _offset, sizePerSubresource);
	TransitionMain(_cmdList->graphic, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	TransitionIntermediate(_cmdList->graphic, D3D12_RESOURCE_STATE_COMMON);
}
