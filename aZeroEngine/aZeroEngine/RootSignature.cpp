#include "RootSignature.h"

void RootSignature::Init(ID3D12Device* device, RootParameters* params, D3D12_ROOT_SIGNATURE_FLAGS flags, UINT numStaticSamplers = 0, 
	D3D12_STATIC_SAMPLER_DESC* staticSamplerDesc = nullptr)
{
	D3D12_ROOT_SIGNATURE_DESC desc{(UINT)params->GetParameterNum(), params->GetParameterData(), numStaticSamplers, staticSamplerDesc, flags};

	Microsoft::WRL::ComPtr<ID3DBlob> serializeBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, serializeBlob.GetAddressOf(), errorBlob.GetAddressOf());

	if (FAILED(hr))
		throw;

	hr = device->CreateRootSignature(0, serializeBlob->GetBufferPointer(), serializeBlob->GetBufferSize(), IID_PPV_ARGS(m_signature.GetAddressOf()));
	if (FAILED(hr))
		throw;
}
