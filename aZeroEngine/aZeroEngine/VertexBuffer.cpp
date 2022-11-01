#include "VertexBuffer.h"
#include "HelperFunctions.h"

VertexBuffer::VertexBuffer(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, int _stride)
	:BaseResource()
{
	state = D3D12_RESOURCE_STATE_COPY_DEST;

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

	Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, rDesc, _cmdList, _data, _size * _stride, _size * _stride);

	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_GENERIC_READ);

	view.BufferLocation = resource->GetGPUVirtualAddress();
	view.SizeInBytes = _size * _stride;
	view.StrideInBytes = _stride;
}

VertexBuffer::~VertexBuffer()
{
	uploadBuffer->Release();
}
