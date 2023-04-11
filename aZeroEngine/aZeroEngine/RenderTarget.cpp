#include "RenderTarget.h"

void RenderTarget::Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height, 
	UINT _channels, DXGI_FORMAT _format, const Vector4& _clearColor, bool _readback)
{
	clearValue.Color[0] = _clearColor.x;
	clearValue.Color[1] = _clearColor.y;
	clearValue.Color[2] = _clearColor.z;
	clearValue.Color[3] = _clearColor.w;

	TextureResource::Init(_device, _width, _height, _channels, _format, D3D12_RESOURCE_STATE_RENDER_TARGET, clearValue, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, _readback);
	//mainResource = Helper::CreateTextureResource(_device, _width, _height, _format,
	//	D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_HEAP_TYPE_DEFAULT, clearValue, mainResourceState);

	handle = _rtvHandle;
	Helper::CreateRTVHandle(_device, gpuOnlyResource, handle.GetCPUHandle(), _format);

	srvHandle = _srvHandle;
	Helper::CreateSRVHandle(_device, gpuOnlyResource, srvHandle.GetCPUHandle(), _format);
}

void RenderTarget::Clear(CommandList& _cmdList)
{
	_cmdList.GetGraphicList()->ClearRenderTargetView(handle.GetCPUHandle(), clearValue.Color, 0, nullptr);
}

void RenderTarget::Init(ID3D12Device* _device, DescriptorHandle _rtvHandle, UINT _width, UINT _height, UINT _channels, DXGI_FORMAT _format, bool _readback)
{
	clearValue.Color[0] = 1;
	clearValue.Color[1] = 1;
	clearValue.Color[2] = 1;
	clearValue.Color[3] = 1;

	//mainResource = Helper::CreateTextureResource(_device, _width, _height, _format,
	//	D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_HEAP_TYPE_DEFAULT, clearValue, mainResourceState);

	TextureResource::Init(_device, _width, _height, _channels, _format, D3D12_RESOURCE_STATE_RENDER_TARGET, 
		clearValue, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, _readback);

	handle = _rtvHandle;
	Helper::CreateRTVHandle(_device, gpuOnlyResource, handle.GetCPUHandle(), _format);
}
