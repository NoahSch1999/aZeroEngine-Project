#include "Sampler.h"

Sampler::Sampler(ID3D12Device* device, DescriptorHandle handle, D3D12_FILTER filter,
	D3D12_TEXTURE_ADDRESS_MODE addressModeU, D3D12_TEXTURE_ADDRESS_MODE addressModeV,
	D3D12_TEXTURE_ADDRESS_MODE addressModeW, D3D12_COMPARISON_FUNC comparisonFunc, 
	int maxAnisotropy, DXM::Vector4 borderColor, float midLodBias, float minLod, float maxLod)
{
	D3D12_SAMPLER_DESC desc = {};
	desc.Filter = filter;
	desc.AddressU = addressModeU;
	desc.AddressV = addressModeV;
	desc.AddressW = addressModeW;
	desc.BorderColor[0] = borderColor.x;
	desc.BorderColor[1] = borderColor.y;
	desc.BorderColor[2] = borderColor.z;
	desc.BorderColor[3] = borderColor.w;
	desc.ComparisonFunc = comparisonFunc;
	desc.MaxAnisotropy = maxAnisotropy;
	desc.MipLODBias = midLodBias;
	desc.MinLOD = minLod;
	desc.MaxLOD = maxLod;

	handle = handle;
	device->CreateSampler(&desc, handle.getCPUHandle());
}

Sampler::Sampler(D3D12_FILTER filter, int shaderRegister, D3D12_SHADER_VISIBILITY shaderVisibility, int registerSpace,
	D3D12_TEXTURE_ADDRESS_MODE addressModeU, D3D12_TEXTURE_ADDRESS_MODE addressModeV, D3D12_TEXTURE_ADDRESS_MODE addressModeW, 
	D3D12_COMPARISON_FUNC comparisonFunc, int maxAnisotropy, DXM::Vector4 borderColor, float midLodBias, float minLod, float maxLod)
{
	m_staticDesc.Filter = filter;
	m_staticDesc.AddressU = addressModeU;
	m_staticDesc.AddressV = addressModeV;
	m_staticDesc.AddressW = addressModeW;
	m_staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	m_staticDesc.ComparisonFunc = comparisonFunc;
	m_staticDesc.MaxAnisotropy = maxAnisotropy;
	m_staticDesc.MipLODBias = midLodBias;
	m_staticDesc.MinLOD = minLod;
	m_staticDesc.MaxLOD = maxLod;
	m_staticDesc.ShaderRegister = shaderRegister;
	m_staticDesc.RegisterSpace = registerSpace;
	m_staticDesc.ShaderVisibility = shaderVisibility;
}

void Sampler::init(ID3D12Device* device, DescriptorHandle handle, D3D12_FILTER filter,
	D3D12_TEXTURE_ADDRESS_MODE addressModeU,
	D3D12_TEXTURE_ADDRESS_MODE addressModeV,
	D3D12_TEXTURE_ADDRESS_MODE addressModeW,
	D3D12_COMPARISON_FUNC comparisonFunc,
	int maxAnisotropy, DXM::Vector4 borderColor,
	float midLodBias, float minLod, float maxLod)
{
	D3D12_SAMPLER_DESC desc = {};
	desc.Filter = filter;
	desc.AddressU = addressModeU;
	desc.AddressV = addressModeV;
	desc.AddressW = addressModeW;
	desc.BorderColor[0] = borderColor.x;
	desc.BorderColor[1] = borderColor.y;
	desc.BorderColor[2] = borderColor.z;
	desc.BorderColor[3] = borderColor.w;
	desc.ComparisonFunc = comparisonFunc;
	desc.MaxAnisotropy = maxAnisotropy;
	desc.MipLODBias = midLodBias;
	desc.MinLOD = minLod;
	desc.MaxLOD = maxLod;

	handle = handle;
	device->CreateSampler(&desc, handle.getCPUHandle());
}
