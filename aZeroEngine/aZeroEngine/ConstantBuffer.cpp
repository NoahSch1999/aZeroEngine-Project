#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer()
	:BaseResource()
{
}

ConstantBuffer::~ConstantBuffer()
{
}

void ConstantBuffer::InitAsCBV(ID3D12Device* _device, const DescriptorHandle& _handle)
{
	handle = _handle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
	view.BufferLocation = mainResource->GetGPUVirtualAddress();
	view.SizeInBytes = sizePerSubresource;
	_device->CreateConstantBufferView(&view, handle.GetCPUHandle());
}

void ConstantBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName)
{
	numElements = _numElements;
	isStatic = true;
	sizePerSubresource = (_numBytes / _numElements + 255) & ~255;

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

	Init<ResourceType::StaticResource>(_device, _cmdList, rDesc, rDesc, _initData);
	mainResource->SetName(_mainResourceName.c_str());

	// clear intermediate somehow
}

void ConstantBuffer::InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName)
{
	numElements = _numElements;
	sizePerSubresource = (_numBytes / _numElements + 255) & ~255; // size per buffer subresource

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

	isTrippleBuffered = _trippleBuffered;

	Init<ResourceType::DynamicResource>(_device, _cmdList, rDesc, rDesc, _initData);

	mainResource->SetName(_mainResourceName.c_str());
	_mainResourceName + L".Intermediate";
	if(intermediateResource != nullptr)
		intermediateResource->SetName(_mainResourceName.c_str());
}