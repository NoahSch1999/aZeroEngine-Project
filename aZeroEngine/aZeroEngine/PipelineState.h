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
	PipelineState() = default;

	/** @brief Releases the internal ID3D12PipelineState* object.
	*/
	~PipelineState() = default;

	/** @brief Initializes the object.
	@param device Device to use when creating the D3D12 resources.
	@param rootSignature RootSignature* describing the root signature for the pipeline state.
	@param inputLayout const InputLayout& describing the input layout.
	@param rasterState RasterState* describing the rasterizer state.
	@param numRenderTargets Num render targets for the pipeline state.
	@param rtvFormats The formats of the render targets.
	@param dsvFormat FORMAT of the swapchain depth stencil.
	@param vsPath Path for the vertex shader.
	@param psPath Path for the pixel shader.
	@param dsPath Path for the domain shader.
	@param hsPath Path for the hull shader.
	@param gsPath Path for the geometry shader.
	@param primitiveType Primitive topology type to be used.
	@return void
	*/
	void Init(ID3D12Device* device, RootSignature* rootSignature, const InputLayout& inputLayout, const RasterState& rasterState, 
		UINT numRenderTargets, DXGI_FORMAT* const rtvFormats, DXGI_FORMAT dsvFormat, const std::wstring& vsPath, const std::wstring& psPath,
		const std::wstring& dsPath, const std::wstring& hsPath, const std::wstring& gsPath, bool enableBlending = true,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	/** @brief Returns a pointer to the internal ID3D12PipelineState* object.
	@return ID3D12PipelineState*
	*/
	ID3D12PipelineState* GetPipelineState() { return m_pipelineState.Get(); }
};

