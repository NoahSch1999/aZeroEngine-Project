#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	init(device, type, maxDescriptors, flags);
}

void DescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.NumDescriptors = maxDescriptors;
	desc.Type = type;
	desc.Flags = flags;
	desc.NodeMask = 0;

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.GetAddressOf()));
	if (FAILED(hr))
		throw;

	m_handle.setHandle(m_heap->GetCPUDescriptorHandleForHeapStart());

	m_type = type;
	m_maxDescriptors = maxDescriptors;
	m_descriptorSize = device->GetDescriptorHandleIncrementSize(type);

	m_descriptorList.init(maxDescriptors);

	if (flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		m_handle.setHandle(m_heap->GetGPUDescriptorHandleForHeapStart());
		m_isShaderVisible = true;
	}
}

DescriptorHandle DescriptorHeap::getDescriptorHandle()
{
	int index = m_descriptorList.lendKey();
	D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle = m_handle.getCPUHandle();
	D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle = m_handle.getGPUHandle();
	startCPUHandle.ptr += index * m_descriptorSize;

	DescriptorHandle newHandle(startCPUHandle, index);

	if (m_isShaderVisible)
	{
		startGPUHandle.ptr += index * m_descriptorSize;
		newHandle.setHandle(startGPUHandle);
	}

	return newHandle;
}

std::vector<DescriptorHandle> DescriptorHeap::getDescriptorHandles(int numHandles)
{
	std::vector<DescriptorHandle>handles(numHandles);
	for (int i = 0; i < numHandles; i++)
	{
		handles[i] = getDescriptorHandle();
	}

	return handles;
}

void DescriptorHeap::freeDescriptorHandle(DescriptorHandle& handle)
{
	int temp = handle.getHeapIndex();
	m_descriptorList.returnKey(temp);
	handle.setHeapIndex(temp);
}
