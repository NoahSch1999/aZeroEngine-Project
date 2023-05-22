#pragma once
#include "RootSignature.h"
#include "RasterState.h"
#include "InputLayout.h"
#include "SwapChain.h"

/** @brief Encapsulates a pipeline state.
*/
class PipelineState
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

public:

	struct PipelineStateDesc
	{
		RootSignature* m_rootSignature = nullptr;
		InputLayout* m_inputLayout = nullptr;
		RasterState* m_rasterState = nullptr;
		DXGI_FORMAT* m_rtvFormats = nullptr;
		UINT m_numRTVFormats = 0;
		DXGI_FORMAT m_dsvFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		std::wstring* m_vShaderPath = nullptr;
		std::wstring* m_pShaderPath = nullptr;
		std::wstring* m_dShaderPath = nullptr;
		std::wstring* m_hShaderPath = nullptr;
		std::wstring* m_gShaderPath = nullptr;
		D3D12_BLEND_DESC* m_blendDesc = nullptr;
		D3D12_DEPTH_STENCIL_DESC* m_dsvDesc = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE m_primitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	};

	PipelineState() = default;

	/** @brief Releases the internal ID3D12PipelineState* object.
	*/
	~PipelineState() = default;

	/** @brief Initializes the object.
	@param device Device to use when creating the D3D12 resources.
	@param description A PipelineStateDesc describing the PipelineState.
	@return void
	*/
	void init(ID3D12Device* device, const PipelineStateDesc& description);

	/** @brief Returns a pointer to the internal ID3D12PipelineState* object.
	@return ID3D12PipelineState*
	*/
	ID3D12PipelineState* getPipelineState() { return m_pipelineState.Get(); }
};

