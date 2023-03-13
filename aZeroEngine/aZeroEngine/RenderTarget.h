#pragma once
#include "BaseResource.h"

class RenderTarget : public BaseResource
{
private:
	DescriptorHandle srvHandle;
	DXGI_FORMAT format;
public:
	RenderTarget() = default;

	virtual ~RenderTarget() {};

	/**Returns a reference to the shader resource view descriptor handle.
	@return DescriptorHandle&
	*/
	constexpr DescriptorHandle& GetSrvHandle() { return srvHandle; }

	void InitBase(ID3D12Device* _device, DescriptorHandle _rtvHandle, DescriptorHandle _srvHandle, UINT _width, UINT _height, DXGI_FORMAT _format, const Vector4& _clearColor)
	{
		format = _format;
		D3D12_RESOURCE_DESC rDesc = {};
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Width = _width;
		rDesc.Height = _height;
		rDesc.DepthOrArraySize = 1; // ?
		rDesc.MipLevels = 1;
		rDesc.Format = _format;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = _format;
		clearValue.Color[0] = _clearColor.x;
		clearValue.Color[1] = _clearColor.y;
		clearValue.Color[2] = _clearColor.z;
		clearValue.Color[3] = _clearColor.w;
		clearValue.DepthStencil.Depth = 1;
		clearValue.DepthStencil.Stencil = 0;

		HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		mainResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		handle = _rtvHandle;
		srvHandle = _srvHandle;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = _format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		_device->CreateRenderTargetView(mainResource, &rtvDesc, handle.GetCPUHandle());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		_device->CreateShaderResourceView(mainResource, &srvDesc, srvHandle.GetCPUHandle());
	}

	void InitBase(ID3D12Device* _device, DescriptorHandle _rtvHandle, UINT _width, UINT _height, DXGI_FORMAT _format)
	{
		format = _format;
		D3D12_RESOURCE_DESC rDesc = {};
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Width = _width;
		rDesc.Height = _height;
		rDesc.DepthOrArraySize = 1; // ?
		rDesc.MipLevels = 1;
		rDesc.Format = _format;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = _format;
		clearValue.Color[0] = 1;
		clearValue.Color[1] = 1;
		clearValue.Color[2] = 1;
		clearValue.Color[3] = 1;
		clearValue.DepthStencil.Depth = 1;
		clearValue.DepthStencil.Stencil = 0;

		HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		handle = _rtvHandle;
		mainResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(D3D12_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = _format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		_device->CreateRenderTargetView(mainResource, &rtvDesc, handle.GetCPUHandle());
	}
};

