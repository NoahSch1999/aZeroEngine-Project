#pragma once
#include "D3D12Include.h"

/** @brief Encapsulates a Rasterizer state.
*/
class RasterState
{
private:
	D3D12_RASTERIZER_DESC m_rasterDescription = {};

public:
	RasterState() = default;
	/** @brief Initializes the object.
	@param fillMode D3D12_FILL_MODE for the RasterState.
	@param cullMode D3D12_CULL_MODE for the RasterState.
	@param frontCCW If the RasterState should use clockwise or counter-clockwise culling. Defaulted to false.
	@param multiSample If the RasterState should use multisampling. Defaulted to true.
	@param antiAliasing If the RasterState should use anti-aliasing. Defaulted to true.
	*/
	RasterState(D3D12_FILL_MODE fillMode, D3D12_CULL_MODE cullMode, bool frontCCW = false, bool multiSample = true, bool antiAliasing = true)
	{
		m_rasterDescription = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_rasterDescription.CullMode = cullMode;
		m_rasterDescription.FillMode = fillMode;
		m_rasterDescription.AntialiasedLineEnable = antiAliasing;
		m_rasterDescription.MultisampleEnable = multiSample;
		m_rasterDescription.FrontCounterClockwise = frontCCW;
		m_rasterDescription.DepthClipEnable = true;
	}

	/** @brief Returns the D3D12_RASTERIZER_DESC for the RasterState.
	@return D3D12_RASTERIZER_DESC.
	*/
	D3D12_RASTERIZER_DESC getDesc() const { return m_rasterDescription; }
};