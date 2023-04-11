#include "ShaderDescriptorHeap.h"

ShaderDescriptorHeap::ShaderDescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors)
{
	ShaderDescriptorHeap::Init(_device, _type, _maxDescriptors);
}

void ShaderDescriptorHeap::Init(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors)
{
	DescriptorHeap::Init(_device, _type, _maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}