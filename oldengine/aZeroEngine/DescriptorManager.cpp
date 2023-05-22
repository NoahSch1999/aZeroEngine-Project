#include "DescriptorManager.h"

DescriptorManager::DescriptorManager(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors, int _numRTVDescriptors, int _numDSVDescriptors)
{
	Init(_device, _numResourceDescriptors, _numSamplerDescriptors, _numRTVDescriptors, _numDSVDescriptors);
}

void DescriptorManager::Init(ID3D12Device* _device, int _numResourceDescriptors, int _numSamplerDescriptors, int _numRTVDescriptors, int _numDSVDescriptors)
{
	m_resourceHeap.init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, _numResourceDescriptors);

	m_samplerHeap.init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, _numSamplerDescriptors);

	m_rtvHeap.init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100);

	m_dsvHeap.init(_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100);

#ifdef _DEBUG
	m_resourceHeap.getHeap()->SetName(L"DescriptorManager Resource Heap");
	m_samplerHeap.getHeap()->SetName(L"DescriptorManager Sampler Heap");
	m_rtvHeap.getHeap()->SetName(L"DescriptorManager RTV Heap");
	m_dsvHeap.getHeap()->SetName(L"DescriptorManager DSV Heap");
#endif // DEBUG
}
