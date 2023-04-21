#pragma once
#include "RootParameter.h"

class RootSignature
{
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> signature;

public:
	RootSignature() = default;
	~RootSignature() = default;

	/** @brief Initializes the root signature with the input parameters, flags, sampler and more.
	@param _device ID3D12Device to use for the resource creations.
	@param _params RootParameters to use for the root signature.
	@param _flags D3D12_ROOT_SIGNATURE_FLAGS to use for the root signature.
	@param _numStaticSamplers Number of D3D12_STATIC_SAMPLER_DESC input as the last argument.
	@param _staticSamplerDesc Pointer to an array of D3D12_STATIC_SAMPLER_DESC to use for the root signature.
	@return void
	*/
	void Init(ID3D12Device* _device, RootParameters* _params, D3D12_ROOT_SIGNATURE_FLAGS _flags, UINT _numStaticSamplers,
		D3D12_STATIC_SAMPLER_DESC* _staticSamplerDesc);

	/** @brief Returns a pointer to the internal ID3D12PipelineState* object.
	@return ID3D12PipelineState*
	*/
	ID3D12RootSignature* GetSignature() { return signature.Get(); }
};

