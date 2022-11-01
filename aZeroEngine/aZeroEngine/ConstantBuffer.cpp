#include "ConstantBuffer.h"
#include "HelperFunctions.h"

ConstantBuffer::ConstantBuffer()
	:BaseResource()
{
}

ConstantBuffer::~ConstantBuffer()
{

}

void ConstantBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name)
{
	isStatic = true;
	size = (_size + 255) & ~255;

	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = size;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, rDesc, _cmdList, _data, size, size);

	state = D3D12_RESOURCE_STATE_COPY_DEST;
	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void ConstantBuffer::InitAsDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name)
{
	isStatic = false;
	size = (_size + 255) & ~255;

	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = size;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Helper::CreateCommitedResourceDynamic(_device, resource, rDesc);

	gpuAddress = resource->GetGPUVirtualAddress();

	resource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
	Update(_data, _size);
}

void ConstantBuffer::InitAsCBV(ID3D12Device* _device)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
	view.BufferLocation = resource->GetGPUVirtualAddress();
	view.SizeInBytes = size;
	_device->CreateConstantBufferView(&view, handle.cpuHandle);
}

void ConstantBuffer::Update(const void* _data, int _size)
{
	if (_size > size)
		throw;
	memcpy(mappedBuffer, _data, _size);
}