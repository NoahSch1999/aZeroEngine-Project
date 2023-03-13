#pragma once
#include "BaseResource.h"

/** @brief A class containing everything necessary for creating a depth stencil view.
* It can be created either as a pure depth stencil or as a depth stencil with a linked shader resource view.
*/
class DepthStencil : public BaseResource
{
private:
	DescriptorHandle srvHandle;
public:


	DepthStencil() = default;
	virtual ~DepthStencil() { }

	/**Returns a reference to the shader resource view descriptor handle.
	@return DescriptorHandle&
	*/
	constexpr DescriptorHandle& GetSrvHandle() { return srvHandle; }

	void InitBase(ID3D12Device* _device, DescriptorHandle _dsvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height)
	{
		D3D12_RESOURCE_DESC rDesc = {};
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Width = _width;
		rDesc.Height = _height;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearValue.Color[0] = 1;
		clearValue.Color[1] = 1;
		clearValue.Color[2] = 1;
		clearValue.Color[3] = 1;
		clearValue.DepthStencil.Depth = 1;
		clearValue.DepthStencil.Stencil = 0;

		HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		mainResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		handle = _dsvHandle;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		_device->CreateDepthStencilView(mainResource, &dsvDesc, handle.GetCPUHandle());

		srvHandle = _srvHandle;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		_device->CreateShaderResourceView(mainResource, &srvDesc, srvHandle.GetCPUHandle());
	}

	void InitBase(ID3D12Device* _device, DescriptorHandle _dsvHandle, UINT _width, UINT _height)
	{
		D3D12_RESOURCE_DESC rDesc = {};
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Width = _width;
		rDesc.Height = _height;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clearValue.Color[0] = 1;
		clearValue.Color[1] = 1;
		clearValue.Color[2] = 1;
		clearValue.Color[3] = 1;
		clearValue.DepthStencil.Depth = 1;
		clearValue.DepthStencil.Stencil = 0;

		// how to avoid creating implicit descriptor heap? replace with placeresource... or atleast provide that option
		HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		mainResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

		handle = _dsvHandle;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		ZeroMemory(&dsvDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		_device->CreateDepthStencilView(mainResource, &dsvDesc, handle.GetCPUHandle());
	}
};

