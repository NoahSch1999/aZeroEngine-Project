#include "ConstantBuffer.h"
#include "HelperFunctions.h"

ConstantBuffer::ConstantBuffer()
	:BaseResource(), isStatic(false), isTrippleBuffered(false)
{
}

ConstantBuffer::~ConstantBuffer()
{
	if (isStatic)
		uploadBuffer->Release();
}

void ConstantBuffer::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, const std::wstring& _name)
{
	isStatic = true;
	sizePerSubresource = (_size + 255) & ~255;
	totalSize = sizePerSubresource;

	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = totalSize;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, rDesc, _cmdList, _data, totalSize, totalSize);

	gpuAddress = resource->GetGPUVirtualAddress();
	resource->SetName(_name.c_str());

	state = D3D12_RESOURCE_STATE_COPY_DEST;
	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void ConstantBuffer::InitAsDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _data, int _size, bool _trippleBuffering, const std::wstring& _name)
{
	sizePerSubresource = (_size + 255) & ~255; // size per buffer subresource
	totalSize = sizePerSubresource;

	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = totalSize;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if (_trippleBuffering)
	{
		totalSize *= 3;
		isTrippleBuffered = true;

		D3D12_RESOURCE_DESC uDesc = rDesc;
		uDesc.Width = totalSize;

		Helper::CreateCommitedResourceStatic(_device, resource, rDesc, uploadBuffer, uDesc, _cmdList, _data, sizePerSubresource, sizePerSubresource);
		uploadBuffer->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
		//memcpy(mappedBuffer, _data, sizePerSubresource);
		//memcpy((char*)mappedBuffer + sizePerSubresource, _data, sizePerSubresource);
		//memcpy((char*)mappedBuffer + sizePerSubresource + sizePerSubresource, _data, sizePerSubresource);
	}
	else
	{
		Helper::CreateCommitedResourceDynamic(_device, resource, rDesc);
		resource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
		//Update(_data, _size);
	}


	gpuAddress = resource->GetGPUVirtualAddress();
	resource->SetName(_name.c_str());
}

void ConstantBuffer::InitAsCBV(ID3D12Device* _device, const DescriptorHandle& _handle)
{
	handle = _handle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
	view.BufferLocation = resource->GetGPUVirtualAddress();
	view.SizeInBytes = sizePerSubresource;
	_device->CreateConstantBufferView(&view, handle.GetCPUHandle());
}

void ConstantBuffer::Update(const void* _data, int _size)
{
	if (_size > sizePerSubresource)
		throw;

	memcpy((char*)mappedBuffer, _data, _size);
}

void ConstantBuffer::Update(CommandList* _cmdList, const void* _data, int _size, int _frameIndex)
{
	if (_size > sizePerSubresource)
		throw;

	memcpy((char*)mappedBuffer + sizePerSubresource * _frameIndex, _data, _size);
	Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_COPY_DEST);

	D3D12_RESOURCE_BARRIER barrier(CD3DX12_RESOURCE_BARRIER::Transition(uploadBuffer, uploadState, D3D12_RESOURCE_STATE_COPY_SOURCE));
	_cmdList->graphic->ResourceBarrier(1, &barrier);
	uploadState = D3D12_RESOURCE_STATE_COPY_SOURCE;

	_cmdList->graphic->CopyBufferRegion(resource, 0, uploadBuffer, sizePerSubresource * _frameIndex, sizePerSubresource);
	Transition(_cmdList->graphic, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	D3D12_RESOURCE_BARRIER barrierTwo(CD3DX12_RESOURCE_BARRIER::Transition(uploadBuffer, uploadState, D3D12_RESOURCE_STATE_COMMON));
	_cmdList->graphic->ResourceBarrier(1, &barrierTwo);
	uploadState = D3D12_RESOURCE_STATE_COMMON;
}