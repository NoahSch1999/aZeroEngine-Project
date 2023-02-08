#include "RenderTarget.h"

RenderTarget::RenderTarget(ID3D12Device* _device, HiddenDescriptorHeap* _heap, UINT _width, UINT _height, DXGI_FORMAT _format)
	:BaseResource()
{
	handle = _heap->GetNewSlot();
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = _format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Buffer = { 0, _width * _height };
	desc.Texture2D = { 0, 0 };
	_device->CreateRenderTargetView(mainResource, &desc, handle.GetCPUHandle());
}

void RenderTarget::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName)
{
	throw; // Not implemented yet
}

void RenderTarget::InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName)
{
	throw; // Not implemented yet
}
