#pragma once
#include "RootParameter.h"

class RootSignature
{
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_signature;

public:
	RootSignature() = default;
	~RootSignature() = default;

	/** @brief Initializes the root signature with the input parameters, flags, sampler and more.
	@param device ID3D12Device to use for the resource creations.
	@param params RootParameters to use for the root signature.
	@param flags D3D12_ROOT_SIGNATURE_FLAGS to use for the root signature.
	@param numStaticSamplers Number of D3D12_STATIC_SAMPLER_DESC input as the last argument.
	@param staticSamplerDesc Pointer to an array of D3D12_STATIC_SAMPLER_DESC to use for the root signature.
	@return void
	*/
	void Init(ID3D12Device* device, RootParameters* params, D3D12_ROOT_SIGNATURE_FLAGS flags, UINT numStaticSamplers,
		D3D12_STATIC_SAMPLER_DESC* staticSamplerDesc);

	/** @brief Returns a pointer to the internal ID3D12PipelineState* object.
	@return ID3D12PipelineState*
	*/
	ID3D12RootSignature* GetSignature() { return m_signature.Get(); }
};

