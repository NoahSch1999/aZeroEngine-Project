#pragma once
#include "ShaderDescriptorHeap.h"
#include "BaseResource.h"
#include "HelperFunctions.h"

class Sampler : public BaseResource
{
public:
	D3D12_STATIC_SAMPLER_DESC staticDesc;
	Sampler(ID3D12Device* _device, DescriptorHandle _handle, D3D12_FILTER _filter,
		D3D12_TEXTURE_ADDRESS_MODE _addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP, 
		D3D12_TEXTURE_ADDRESS_MODE _addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE _addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC _comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int _maxAnisotropy = 16, Vector4 _borderColor = { 1,1,1,1 }, 
		float _midLodBias = 0, float _minLod = 0, float _maxLod = D3D12_FLOAT32_MAX);

	Sampler(D3D12_FILTER _filter, 
		int _shaderRegister, D3D12_SHADER_VISIBILITY _shaderVisibility = D3D12_SHADER_VISIBILITY_ALL, int _registerSpace = 0,
		D3D12_TEXTURE_ADDRESS_MODE _addressModeU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE _addressModeV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE _addressModeW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_COMPARISON_FUNC _comparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
		int _maxAnisotropy = 16, Vector4 _borderColor = { 1,1,1,1 },
		float _midLodBias = 0, float _minLod = 0, float _maxLod = D3D12_FLOAT32_MAX); // _maxLod has to be above 0 for some reason.

	~Sampler();
};

