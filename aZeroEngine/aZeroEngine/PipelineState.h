#pragma once
#include "RootSignature.h"
#include "RasterState.h"
#include "InputLayout.h"
#include "SwapChain.h"

class PipelineState
{
public:
	ID3D12PipelineState* pipelineState;

	PipelineState();
	~PipelineState();

	void Init(ID3D12Device* _device, RootSignature* _rootSignature, InputLayout* _inputLayout, RasterState* _rasterState, int _numRenderTargets, DXGI_FORMAT _rtvFormat,
		DXGI_FORMAT _dsvFormat, const std::wstring& _vsPath, const std::wstring& _psPath,
		const std::wstring& _dsPath, const std::wstring& _hsPath, const std::wstring& _gsPath,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE _primitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
};

