#include "StructuredBuffer.h"

void StructuredBuffer::Update(int _elementIndex, void* _data, int _size)
{
	if (_size > sizePerSubresource)
		throw;

	memcpy((char*)mappedBuffer + _elementIndex * sizeOfElement, _data, _size);
}

D3D12_GPU_VIRTUAL_ADDRESS StructuredBuffer::GetGPUAddress(int _frameIndex)
{
	return gpuAddress + sizePerSubresource * _frameIndex;
}

void StructuredBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName)
{
	throw; // Not implemented yet
}

void StructuredBuffer::InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName)
{
	numElements = _numElements;
	sizeOfElement = _numBytes / _numElements;
	sizePerSubresource = _numBytes;
	isTrippleBuffered = false;

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

	//D3D12_RESOURCE_DESC iDesc = rDesc;

	Init<ResourceType::DynamicResource>(_device, _cmdList, rDesc, rDesc, nullptr);
}
