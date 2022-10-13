#include "DescriptorHeap.h"

DescriptorHeap::DescriptorHeap(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, int _maxDescriptors, bool _shaderVisable, const std::wstring& _name)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.NumDescriptors = _maxDescriptors;
	desc.Type = _type;
	desc.Flags = _shaderVisable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
	if (FAILED(hr))
		throw;

	cpuAddress = heap->GetCPUDescriptorHandleForHeapStart();
	gpuAddress = heap->GetGPUDescriptorHandleForHeapStart();
	descriptorSize = _device->GetDescriptorHandleIncrementSize(_type);

	type = _type;
	maxDescriptors = _maxDescriptors;
	numDescriptors = 0;
	currentIndex = 0;

	heap->SetName(_name.c_str());
}

DescriptorHeap::~DescriptorHeap()
{
	heap->Release();
}
