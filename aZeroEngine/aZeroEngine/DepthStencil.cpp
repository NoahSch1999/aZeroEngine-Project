#include "DepthStencil.h"

void DepthStencil::Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height)
{
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Color[0] = 1;
	clearValue.Color[1] = 1;
	clearValue.Color[2] = 1;
	clearValue.Color[3] = 1;
	clearValue.DepthStencil.Depth = 1;
	clearValue.DepthStencil.Stencil = 0;

	TextureResource::Init(_device, _width, _height, 0, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_STATE_DEPTH_WRITE, clearValue, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	//mainResource = Helper::CreateTextureResource(_device, _width, _height, DXGI_FORMAT_D24_UNORM_S8_UINT,
	//	D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_HEAP_TYPE_DEFAULT, clearValue, mainResourceState);

	handle = _dsvHandle;
	Helper::CreateDSVHandle(_device, gpuOnlyResource, handle.GetCPUHandle(), DXGI_FORMAT_D24_UNORM_S8_UINT);

	srvHandle = _srvHandle;
	Helper::CreateSRVHandle(_device, gpuOnlyResource, srvHandle.GetCPUHandle(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);

	gpuOnlyResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

void DepthStencil::Init(ID3D12Device* _device, DescriptorHandle _dsvHandle, UINT _width, UINT _height)
{
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Color[0] = 1;
	clearValue.Color[1] = 1;
	clearValue.Color[2] = 1;
	clearValue.Color[3] = 1;
	clearValue.DepthStencil.Depth = 1;
	clearValue.DepthStencil.Stencil = 0;

	TextureResource::Init(_device, _width, _height, 0, DXGI_FORMAT_D24_UNORM_S8_UINT, D3D12_RESOURCE_STATE_DEPTH_WRITE, clearValue, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	//mainResource = Helper::CreateTextureResource(_device, _width, _height, DXGI_FORMAT_D24_UNORM_S8_UINT,
	//	D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_HEAP_TYPE_DEFAULT, clearValue, mainResourceState);
	//
	handle = _dsvHandle;

	Helper::CreateDSVHandle(_device, gpuOnlyResource, handle.GetCPUHandle(), DXGI_FORMAT_D24_UNORM_S8_UINT);

	gpuOnlyResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

void DepthStencil::Clear(CommandList& _cmdList)
{
	_cmdList.GetGraphicList()->ClearDepthStencilView(handle.GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
}
