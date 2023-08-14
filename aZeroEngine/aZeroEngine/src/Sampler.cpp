#include "Sampler.h"

Sampler::Sampler(ID3D12Device* device, DescriptorManager& descriptorManager, D3D12_FILTER filter,
	D3D12_TEXTURE_ADDRESS_MODE addressModeU, D3D12_TEXTURE_ADDRESS_MODE addressModeV,
	D3D12_TEXTURE_ADDRESS_MODE addressModeW, D3D12_COMPARISON_FUNC comparisonFunc, 
	int maxAnisotropy, DXM::Vector4 borderColor, float midLodBias, float minLod, float maxLod)
	:m_descriptorManager(&descriptorManager)
{
	m_desc.Filter = filter;
	m_desc.AddressU = addressModeU;
	m_desc.AddressV = addressModeV;
	m_desc.AddressW = addressModeW;
	m_desc.BorderColor[0] = borderColor.x;
	m_desc.BorderColor[1] = borderColor.y;
	m_desc.BorderColor[2] = borderColor.z;
	m_desc.BorderColor[3] = borderColor.w;
	m_desc.ComparisonFunc = comparisonFunc;
	m_desc.MaxAnisotropy = maxAnisotropy;
	m_desc.MipLODBias = midLodBias;
	m_desc.MinLOD = minLod;
	m_desc.MaxLOD = maxLod;

	m_handle = m_descriptorManager->getSamplerDescriptor();
	device->CreateSampler(&m_desc, m_handle.getCPUHandle());
}

Sampler::Sampler(const Sampler& other)
{
	m_descriptorManager = other.m_descriptorManager;
	m_desc = other.m_desc;

	m_handle = m_descriptorManager->getSamplerDescriptor();

	ComPtr<ID3D12Device> device = nullptr;
	m_descriptorManager->getSamplerHeap()->GetDevice(IID_PPV_ARGS(device.GetAddressOf()));

	device->CreateSampler(&m_desc, m_handle.getCPUHandle());
}

Sampler& Sampler::operator=(const Sampler& other)
{
	if (this != &other)
	{
		m_descriptorManager = other.m_descriptorManager;
		m_desc = other.m_desc;

		m_handle = m_descriptorManager->getSamplerDescriptor();

		ComPtr<ID3D12Device> device = nullptr;
		m_descriptorManager->getSamplerHeap()->GetDevice(IID_PPV_ARGS(device.GetAddressOf()));

		device->CreateSampler(&m_desc, m_handle.getCPUHandle());
	}

	return *this;
}

Sampler::Sampler(Sampler&& other) noexcept
{
	m_handle = other.m_handle;
	m_descriptorManager = other.m_descriptorManager;
	m_desc = other.m_desc;

	other.m_handle.setHeapIndex(-1);
}

Sampler& Sampler::operator=(Sampler&& other) noexcept
{
	if (this != &other)
	{
		m_handle = other.m_handle;
		m_descriptorManager = other.m_descriptorManager;
		m_desc = other.m_desc;

		other.m_handle.setHeapIndex(-1);
	}
	return *this;
}