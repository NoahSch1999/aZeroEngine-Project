#pragma once
#include "D3D12Include.h"
#include <d3dcompiler.h>
#include "HelperFunctions.h"

class ComputePipelineState
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
public:
	ComputePipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature, const std::wstring fileName)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob = Helper::LoadBlobFromFile(fileName);

		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.CS = { reinterpret_cast<BYTE*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize() };
		desc.pRootSignature = rootSignature;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		if (FAILED(device->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_pipelineState.GetAddressOf()))))
			throw;
	}

	ID3D12PipelineState* getPipelineState() { return m_pipelineState.Get(); }
};