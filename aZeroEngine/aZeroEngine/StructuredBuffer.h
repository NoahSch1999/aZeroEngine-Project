#pragma once
#include "BaseResource.h"
#include "ShaderDescriptorHeap.h"
#include "CommandList.h"
#include "HelperFunctions.h"

class StructuredBuffer : public BaseResource
{
private:
	int numElements;
	int sizeOfElement;
	int sizePerSubresource;
	bool isTrippleBuffered = false;

	int totalSize;

	ID3D12Resource* uploadBuffer;
	void* mappedBuffer;

public:
	StructuredBuffer()
		:BaseResource()
	{

	}

	~StructuredBuffer()
	{
		uploadBuffer->Release();
	}

	void Init(ID3D12Device* _device, CommandList* _cmdList, int _numElements, int _sizeOfElement, bool _trippleBuffering = false)
	{
		numElements = _numElements;
		sizeOfElement = _sizeOfElement;
		sizePerSubresource = _numElements * _sizeOfElement;
		isTrippleBuffered = _trippleBuffering;
		totalSize = sizePerSubresource;

		if (isTrippleBuffered)
		{
			totalSize = sizePerSubresource * 3;
		}

		D3D12_RESOURCE_DESC rDesc;
		ZeroMemory(&rDesc, sizeof(D3D12_RESOURCE_DESC));
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Width = totalSize;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;

		Helper::CreateCommitedResourceDynamic(_device, resource, rDesc);
		gpuAddress = resource->GetGPUVirtualAddress();

		resource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
	}

	void Update(int _elementIndex, void* _data, int _size)
	{
		if (_size > totalSize)
			throw;

		memcpy((char*)mappedBuffer + _elementIndex * sizeOfElement, _data, _size);
	}

	void Update(int _frameIndex, int _elementIndex, void* _data, int _size)
	{
		if (_size > totalSize)
			throw;

		memcpy((char*)mappedBuffer + (sizePerSubresource * _frameIndex) + (_elementIndex * sizeOfElement), _data, _size);
	}

	using BaseResource::GetGPUAddress;
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(int _frameIndex);
};

