#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags)
{
	Init(_device, _type, _maxDescriptors, _flags);
}

void DescriptorHeap::Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS _flags)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.NumDescriptors = _maxDescriptors;
	desc.Type = _type;
	desc.Flags = _flags;
	desc.NodeMask = 0;

	HRESULT hr = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));
	if (FAILED(hr))
		throw;

	handle.SetHandle(heap->GetCPUDescriptorHandleForHeapStart());

	type = _type;
	maxDescriptors = _maxDescriptors;
	descriptorSize = _device->GetDescriptorHandleIncrementSize(_type);

	descriptorList.Init(_maxDescriptors);

	if (_flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		handle.SetHandle(heap->GetGPUDescriptorHandleForHeapStart());
		isShaderVisible = true;
	}
}

DescriptorHandle DescriptorHeap::GetDescriptorHandle()
{
	int index = descriptorList.LendKey();
	D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle = handle.GetCPUHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle = handle.GetGPUHandle();
	startCPUHandle.ptr += index * descriptorSize;

	DescriptorHandle newHandle(startCPUHandle, index);

	if (isShaderVisible)
	{
		startGPUHandle.ptr += index * descriptorSize;
		newHandle.SetHandle(startGPUHandle);
	}

	return newHandle;
}

std::vector<DescriptorHandle> DescriptorHeap::GetDescriptorHandles(int _numHandles)
{
	std::vector<DescriptorHandle>handles(_numHandles);
	for (int i = 0; i < _numHandles; i++)
	{
		handles[i] = GetDescriptorHandle();
	}

	return handles;
}

void DescriptorHeap::FreeDescriptorHandle(DescriptorHandle& _handle)
{
	int temp = _handle.GetHeapIndex();
	descriptorList.ReturnKey(temp);
	_handle.SetHeapIndex(temp);
}
