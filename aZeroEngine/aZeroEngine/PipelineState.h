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
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
public:
	PipelineState() = default;

	/** @brief Releases the internal ID3D12PipelineState* object.
	*/
	~PipelineState() = default;

	/** @brief Initializes the object.
	@param _device Device to use when creating the D3D12 resources.
	@param _rootSignature RootSignature* describing the root signature for the pipeline state.
	@param _inputLayout const InputLayout& describing the input layout.
	@param _rasterState RasterState* describing the rasterizer state.
	@param _numRenderTargets Num render targets for the pipeline state.
	@param _rtvFormat FORMAT of the back buffers.
	@param _dsvFormat FORMAT of the swapchain depth stencil.
	@param _vsPath Path for the vertex shader.
	@param _psPath Path for the pixel shader.
	@param _dsPath Path for the domain shader.
	@param _hsPath Path for the hull shader.
	@param _gsPath Path for the geometry shader.
	@param _primitiveType Primitive topology type to be used.
	@return void
	*/
	void Init(ID3D12Device* _device, RootSignature* _rootSignature, const InputLayout& _inputLayout, const RasterState& _rasterState, int _numRenderTargets, DXGI_FORMAT _rtvFormat,
		DXGI_FORMAT _dsvFormat, const std::wstring& _vsPath, const std::wstring& _psPath,
		const std::wstring& _dsPath, const std::wstring& _hsPath, const std::wstring& _gsPath, bool _test = false, bool _enableBlending = true,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE _primitiveType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	/** @brief Returns a pointer to the internal ID3D12PipelineState* object.
	@return ID3D12PipelineState*
	*/
	ID3D12PipelineState* GetPipelineState() { return pipelineState.Get(); }
};

