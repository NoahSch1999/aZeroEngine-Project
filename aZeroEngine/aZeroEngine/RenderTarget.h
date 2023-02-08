#pragma once
#include "BaseResource.h"
#include "HiddenDescriptorHeap.h"
#include "CommandList.h"

class RenderTarget : public BaseResource
{
public:
	RenderTarget() = default;
	RenderTarget(ID3D12Device* _device, HiddenDescriptorHeap* _heap, UINT _width, UINT _height, DXGI_FORMAT _format);
	virtual ~RenderTarget() {};

	// Inherited via BaseResource
	// Both have to be defined
	virtual void InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName) override;
	virtual void InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName) override;
};

