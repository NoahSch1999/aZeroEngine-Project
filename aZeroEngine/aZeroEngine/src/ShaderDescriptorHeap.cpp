#include "ShaderDescriptorHeap.h"

ShaderDescriptorHeap::ShaderDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors)
{
	ShaderDescriptorHeap::init(device, type, maxDescriptors);
}

void ShaderDescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, int maxDescriptors)
{
	DescriptorHeap::init(device, type, maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}