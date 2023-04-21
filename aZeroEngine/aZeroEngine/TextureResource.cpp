#include "TextureResource.h"

void TextureResource::Init(ID3D12Device* _device, Microsoft::WRL::ComPtr<ID3D12Resource>& _uploadResource, CommandList& _transitionList, CommandList& _copyList, void* _data, int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState)
{
	width = _width;
	height = _height;
	format = _format;
	bytePerTexel = _bytePerTexel;
	gpuOnlyResourceState = _initState;

	Helper::CreateTextureResource(_device, _transitionList, _copyList, gpuOnlyResource, _uploadResource, _data, width, height, _bytePerTexel, _format, gpuOnlyResourceState);

#ifdef _DEBUG
	gpuOnlyResource->SetName(L"Main Texture");
	_uploadResource->SetName(L"Temp Upload Texture");
#endif // _DEBUG
}

void TextureResource::Init(ID3D12Device* _device, int _width, int _height, int _bytePerTexel, DXGI_FORMAT _format, D3D12_RESOURCE_STATES _initState, D3D12_CLEAR_VALUE _clearValue, D3D12_RESOURCE_FLAGS _flags, bool _readback)
{
	width = _width;
	height = _height;
	format = _format;
	bytePerTexel = _bytePerTexel;
	gpuOnlyResourceState = _initState;

	gpuOnlyResource = Helper::CreateTextureResource(_device, desc, _width, _height, _format, _flags, D3D12_HEAP_TYPE_DEFAULT, _clearValue, gpuOnlyResourceState);

	if (_readback)
	{
		rowPitch = ((_width * _bytePerTexel + 128) / 256) * 256;
		readbackResource = Helper::CreateReadbackBuffer(_device, rowPitch, _height);

#ifdef _DEBUG
		readbackResource->SetName(L"Readback Texture");
#endif // _DEBUG
	}

#ifdef _DEBUG
	gpuOnlyResource->SetName(L"Main Texture");
#endif // _DEBUG
}

void TextureResource::Transition(CommandList& _cmdList, D3D12_RESOURCE_STATES _newState)
{
	if (gpuOnlyResourceState == _newState)
		return;

	D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(gpuOnlyResource.Get(), gpuOnlyResourceState, _newState);
	_cmdList.GetGraphicList()->ResourceBarrier(1, &r);

	gpuOnlyResourceState = _newState;
}
