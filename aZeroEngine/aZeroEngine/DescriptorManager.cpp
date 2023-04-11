#include "DescriptorManager.h"

DescriptorManager::DescriptorManager(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors, int _numRTVDescriptors, int _numDSVDescriptors)
{
	Init(_device, _numResourceDescriptors, _numSamplerDescriptors, _numRTVDescriptors, _numDSVDescriptors);
}

void DescriptorManager::Init(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors, int _numRTVDescriptors, int _numDSVDescriptors)
{
	resourceHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _numResourceDescriptors);

	samplerHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _numSamplerDescriptors);

	rtvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100);

	dsvHeap.Init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100);

#ifdef _DEBUG
	resourceHeap.GetHeap()->SetName(L"DescriptorManager Resource Heap");
	samplerHeap.GetHeap()->SetName(L"DescriptorManager Sampler Heap");
	rtvHeap.GetHeap()->SetName(L"DescriptorManager RTV Heap");
	dsvHeap.GetHeap()->SetName(L"DescriptorManager DSV Heap");
#endif // DEBUG
}
