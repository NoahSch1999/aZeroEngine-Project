#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride)
	:BaseResource()
{

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

	D3D12_HEAP_PROPERTIES heapProps;
	ZeroMemory(&heapProps, sizeof(heapProps));
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // L0?
	heapProps.VisibleNodeMask = 0;

	HRESULT hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource));
	if (FAILED(hr))
		throw;

	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	hr = _device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
	if (FAILED(hr))
		throw;

	D3D12_SUBRESOURCE_DATA subData = {};
	subData.pData = _data;
	subData.RowPitch = _size * _stride;
	subData.SlicePitch = subData.RowPitch;

	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_COPY_DEST);
	UpdateSubresources(_cmdList->graphic, resource, uploadBuffer, 0, 0, 1, &subData);
	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_GENERIC_READ);

	view.BufferLocation = resource->GetGPUVirtualAddress();
	view.SizeInBytes = _size * _stride;
	view.StrideInBytes = _stride;
}

VertexBuffer::~VertexBuffer()
{
	uploadBuffer->Release();
}
