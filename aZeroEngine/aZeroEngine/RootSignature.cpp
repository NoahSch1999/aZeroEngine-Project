#include "RootSignature.h"

RootSignature::RootSignature()
{
}

RootSignature::~RootSignature()
{
	signature->Release();
}

void RootSignature::Initialize(ID3D12Device* _device, RootParameters* _params, D3D12_ROOT_SIGNATURE_FLAGS _flags, UINT _numStaticSamplers = 0, 
	D3D12_STATIC_SAMPLER_DESC* _staticSamplerDesc = nullptr)
{
	D3D12_ROOT_SIGNATURE_DESC desc{(UINT)_params->parameters.size(), _params->parameters.data(), _numStaticSamplers, _staticSamplerDesc, _flags };

	// might cause mem leak
	ID3DBlob* serializeBlob;
	ID3DBlob* errorBlob;
	//

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &serializeBlob, &errorBlob);
	if (FAILED(hr))
		throw;

	hr = _device->CreateRootSignature(0, serializeBlob->GetBufferPointer(), serializeBlob->GetBufferSize(), IID_PPV_ARGS(&signature));
	if (FAILED(hr))
		throw;
}
