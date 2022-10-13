#pragma once
#include "BaseResource.h"
#include "HiddenDescriptorHeap.h"

class RenderTarget : public BaseResource
{
public:
	RenderTarget() = default;
	RenderTarget(ID3D12Device* _device, HiddenDescriptorHeap* _heap, UINT _width, UINT _height, DXGI_FORMAT _format);
};

