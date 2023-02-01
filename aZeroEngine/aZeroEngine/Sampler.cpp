#include "Sampler.h"

Sampler::Sampler(ID3D12Device* _device, DescriptorHandle _handle, D3D12_FILTER _filter,
	D3D12_TEXTURE_ADDRESS_MODE _addressModeU, D3D12_TEXTURE_ADDRESS_MODE _addressModeV,
	D3D12_TEXTURE_ADDRESS_MODE _addressModeW, D3D12_COMPARISON_FUNC _comparisonFunc, 
	int _maxAnisotropy, Vector4 _borderColor, float _midLodBias, float _minLod, float _maxLod)
{
	D3D12_SAMPLER_DESC desc = {};
	desc.Filter = _filter;
	desc.AddressU = _addressModeU;
	desc.AddressV = _addressModeV;
	desc.AddressW = _addressModeW;
	desc.BorderColor[0] = _borderColor.x;
	desc.BorderColor[1] = _borderColor.y;
	desc.BorderColor[2] = _borderColor.z;
	desc.BorderColor[3] = _borderColor.w;
	desc.ComparisonFunc = _comparisonFunc;
	desc.MaxAnisotropy = _maxAnisotropy;
	desc.MipLODBias = _midLodBias;
	desc.MinLOD = _minLod;
	desc.MaxLOD = _maxLod;

	handle = _handle;
	_device->CreateSampler(&desc, handle.GetCPUHandle());
}

Sampler::Sampler(D3D12_FILTER _filter, int _shaderRegister, D3D12_SHADER_VISIBILITY _shaderVisibility, int _registerSpace, D3D12_TEXTURE_ADDRESS_MODE _addressModeU, D3D12_TEXTURE_ADDRESS_MODE _addressModeV, D3D12_TEXTURE_ADDRESS_MODE _addressModeW, D3D12_COMPARISON_FUNC _comparisonFunc, int _maxAnisotropy, Vector4 _borderColor, float _midLodBias, float _minLod, float _maxLod)
{
	staticDesc.Filter = _filter;
	staticDesc.AddressU = _addressModeU;
	staticDesc.AddressV = _addressModeV;
	staticDesc.AddressW = _addressModeW;
	staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	staticDesc.ComparisonFunc = _comparisonFunc;
	staticDesc.MaxAnisotropy = _maxAnisotropy;
	staticDesc.MipLODBias = _midLodBias;
	staticDesc.MinLOD = _minLod;
	staticDesc.MaxLOD = _maxLod;
	staticDesc.ShaderRegister = _shaderRegister;
	staticDesc.RegisterSpace = _registerSpace;
	staticDesc.ShaderVisibility = _shaderVisibility;
}

Sampler::~Sampler()
{
}
