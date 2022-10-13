#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(ID3D12Device* _device, ShaderDescriptorHeap* _heap, CommandList* _cmdList, void* _data, int _size, bool _static, const std::wstring& _name)
	:BaseResource()
{
	D3D12_RESOURCE_DESC rDesc = {};
	ZeroMemory(&rDesc, sizeof(rDesc));
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rDesc.Width = (_size + 255) & ~255;
	rDesc.Height = 1;
	rDesc.DepthOrArraySize = 1;
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_UNKNOWN;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES props = {};
	HRESULT hr;
	if (_static)
	{
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
		if (FAILED(hr))
			throw;

		// copy to
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));
		if (FAILED(hr))
			throw;

		D3D12_SUBRESOURCE_DATA sData = {};
		sData.pData = _data;
		sData.RowPitch = (_size + 255) & ~255;
		sData.SlicePitch = sData.RowPitch;

		UpdateSubresources(_cmdList->graphic, resource, uploadBuffer, 0, 0, 1, &sData);

		D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
		view.BufferLocation = resource->GetGPUVirtualAddress();
		view.SizeInBytes = (_size + 255) & ~255;

		handle = _heap->GetNewDescriptorHandle(1);
		_device->CreateConstantBufferView(&view, handle.cpuHandle);

		state = D3D12_RESOURCE_STATE_COPY_DEST;
		Transition(_cmdList->graphic, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
	else
	{
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
		if (FAILED(hr))
			throw;

		D3D12_CONSTANT_BUFFER_VIEW_DESC view = {};
		view.BufferLocation = resource->GetGPUVirtualAddress();
		view.SizeInBytes = (_size + 255) & ~255;

		gpuAddress = resource->GetGPUVirtualAddress();

		handle = _heap->GetNewDescriptorHandle(1);
		_device->CreateConstantBufferView(&view, handle.cpuHandle);

		dataSize = _size;
		resource->Map(0, NULL, reinterpret_cast<void**>(&mappedBuffer));
		Update(_data, _size);
	}

	resource->SetName(_name.c_str());
	isStatic = _static;
}

ConstantBuffer::ConstantBuffer(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, void* _data, int _size, bool _static, const std::wstring& _name)
	:BaseResource()
{
	
}

ConstantBuffer::~ConstantBuffer()
{

}

void ConstantBuffer::Update(const void* data, int size)
{
	if (size > dataSize)
		throw;
	memcpy(mappedBuffer, data, size);
}