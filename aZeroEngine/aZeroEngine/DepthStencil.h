#pragma once
#include "BaseResource.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

class DepthStencil : public BaseResource
{
public:
	DepthStencil(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, UINT _width, UINT _height, DXGI_FORMAT _format);
	void ReInit(ID3D12Device* _device, CommandList* _cmdList, UINT _width, UINT _height, DXGI_FORMAT _format);
	~DepthStencil();
};

