#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

/** @brief Encapsulates a constant buffer.
Can be either static or dynamic (single or tripple-buffered).
*/
class ConstantBuffer : public BaseResource
{
public:

	ConstantBuffer() = default;
	virtual ~ConstantBuffer() {};

	/**Initiates the resource as a Constant Buffer View with the input DescriptorHandle and asigns the input DescriptorHandle to the internal resource handle.
	@param _device Device to use when creating the view.
	@param _handle Handle to copy to the internal handle.
	@return void
	*/
	void InitAsCBV(ID3D12Device* _device, const DescriptorHandle& _handle)
	{
		handle = _handle;
		D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
		view.BufferLocation = mainResource->GetGPUVirtualAddress();
		view.SizeInBytes = sizePerSubresource;
		_device->CreateConstantBufferView(&view, handle.GetCPUHandle());
	}


	void InitBase(ID3D12Device* _device, CommandList& _copyList, void* _initData, int _numBytes, bool _dynamic = false, int _tripple = false)
	{
		sizePerSubresource = (_numBytes + 255) & ~255; // size per buffer subresource

		D3D12_RESOURCE_DESC rDesc = {};
		ZeroMemory(&rDesc, sizeof(rDesc));
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Width = sizePerSubresource;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		isTrippleBuffered = _tripple;
		if (isTrippleBuffered)
			rDesc.Width = rDesc.Width * 3;

		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&intermediateResource));
		if (FAILED(hr))
			throw;

		intermediateResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _initData;
		sData.RowPitch = sizePerSubresource;
		sData.SlicePitch = sizePerSubresource;

		mainResourceState = D3D12_RESOURCE_STATE_COMMON;
		intermediateResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		UpdateSubresources(_copyList.GetGraphicList(), mainResource, intermediateResource, 0, 0, 1, &sData);

		gpuAddress = mainResource->GetGPUVirtualAddress();
	}
};

