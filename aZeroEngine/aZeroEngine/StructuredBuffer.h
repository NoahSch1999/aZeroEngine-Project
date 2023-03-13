#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"

/** @brief Encapsulates a structured buffer resource.
*/
class StructuredBuffer : public BaseResource
{
private:
	int sizeOfElement = -1;
	int numElements = -1;

public:
	StructuredBuffer()
		:BaseResource()
	{

	}

	virtual ~StructuredBuffer(){ }

	void InitBase(ID3D12Device* _device, CommandList& _copyList, void*& _initData, int _numBytes, int _numElements, bool _dynamic = true, bool _tripple = true)
	{
		numElements = _numElements;
		sizeOfElement = _numBytes / _numElements;
		sizePerSubresource = _numBytes;

		D3D12_RESOURCE_DESC rDesc;
		ZeroMemory(&rDesc, sizeof(D3D12_RESOURCE_DESC));
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Width = sizePerSubresource;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;

		if (_initData == nullptr)
		{
			_initData = new char[_numBytes]; // mem leak
			ZeroMemory(_initData, _numBytes);
		}

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mainResource));
		if (FAILED(hr))
			throw;

		mainResourceState = D3D12_RESOURCE_STATE_COMMON;

		isTrippleBuffered = _tripple;
		if (isTrippleBuffered)
			rDesc.Width = rDesc.Width * 3;

		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&intermediateResource));
		if (FAILED(hr))
			throw;

		intermediateResource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));

		intermediateResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _initData;
		sData.RowPitch = sizePerSubresource;
		sData.SlicePitch = sizePerSubresource;

		UpdateSubresources(_copyList.GetGraphicList(), mainResource, intermediateResource, 0, 0, 1, &sData);

		gpuAddress = mainResource->GetGPUVirtualAddress();
	}
};