#pragma once
#include "RootParameter.h"

class RootSignature
{
public:
	ID3D12RootSignature* signature;
	RootSignature();
	~RootSignature();

	void Initialize(ID3D12Device* _device, RootParameters* _params, D3D12_ROOT_SIGNATURE_FLAGS _flags, UINT _numStaticSamplers,
		D3D12_STATIC_SAMPLER_DESC* _staticSamplerDesc);
};

