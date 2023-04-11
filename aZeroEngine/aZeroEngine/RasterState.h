#pragma once
#include "D3D12Include.h"

/** @brief Encapsulates a Rasterizer state.
*/
class RasterState
{
private:
	D3D12_RASTERIZER_DESC desc = {};

public:
	RasterState() = default;
	/** @brief Initializes the object.
	@param _fillMode D3D12_FILL_MODE for the RasterState.
	@param _cullMode D3D12_CULL_MODE for the RasterState.
	@param _frontCCW If the RasterState should use clockwise or counter-clockwise culling. Defaulted to false.
	@param _multiSample If the RasterState should use multisampling. Defaulted to true.
	@param _antiAliasing If the RasterState should use anti-aliasing. Defaulted to true.
	*/
	RasterState(D3D12_FILL_MODE _fillMode, D3D12_CULL_MODE _cullMode, bool _frontCCW = false, bool _multiSample = true, bool _antiAliasing = true)
	{
		desc.CullMode = _cullMode;
		desc.FillMode = _fillMode;
		desc.AntialiasedLineEnable = _antiAliasing;
		desc.MultisampleEnable = _multiSample;
		desc.FrontCounterClockwise = _frontCCW;
	}

	/** @brief Returns the D3D12_RASTERIZER_DESC for the RasterState.
	@return D3D12_RASTERIZER_DESC.
	*/
	D3D12_RASTERIZER_DESC GetDesc() const { return desc; }

	/** @brief Sets the D3D12_RASTERIZER_DESC for the RasterState.
	@param _desc D3D12_RASTERIZER_DESC object.
	*/
	void SetDesc(const D3D12_RASTERIZER_DESC& _desc) { desc = _desc; }

};