#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
	:BaseResource()
{
}

VertexBuffer::VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride)
	:BaseResource()
{
	mainResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_DESC rDesc;
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	rDesc.Width = _size * _stride;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Helper::CreateCommitedResourceStatic(_device, mainResource, rDesc, intermediateResource, rDesc, _cmdList, _data, _size * _stride, _size * _stride);

	TransitionMain(_cmdList->GetGraphicList(), D3D12_RESOURCE_STATE_GENERIC_READ);

	view.BufferLocation = mainResource->GetGPUVirtualAddress();
	view.SizeInBytes = _size * _stride;
	view.StrideInBytes = _stride;

	numVertices = _size;
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName)
{
	isStatic = true;
	isTrippleBuffered = false;
	mainResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	numVertices = _numElements;
	int stride = _numBytes / _numElements;

	sizePerSubresource = numVertices * stride;

	D3D12_RESOURCE_DESC rDesc;
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	rDesc.Width = sizePerSubresource;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Init<RESOURCETYPE::STATIC>(_device, _cmdList, rDesc, rDesc, _initData);

	view.BufferLocation = mainResource->GetGPUVirtualAddress();
	view.SizeInBytes = sizePerSubresource;
	view.StrideInBytes = stride;
}

void VertexBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, const std::wstring& _mainResourceName)
{
	throw;
}

void VertexBuffer::InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, bool _trippleBuffered, const std::wstring& _mainResourceName)
{
	throw;
}