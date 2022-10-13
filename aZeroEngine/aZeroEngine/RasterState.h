#pragma once
#include "D3D12Include.h"

class RasterState
{
public:
	RasterState(D3D12_FILL_MODE _fillMode, D3D12_CULL_MODE _cullMode, bool _frontCCW = false, bool _multiSample = true, bool _antiAliasing = true)
	{
		desc.CullMode = _cullMode;
		desc.FillMode = _fillMode;
		desc.AntialiasedLineEnable = _antiAliasing;
		desc.MultisampleEnable = _multiSample;
		desc.FrontCounterClockwise = _frontCCW;
	}

	D3D12_RASTERIZER_DESC desc = {};
};