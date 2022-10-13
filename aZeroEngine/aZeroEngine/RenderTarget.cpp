#include "RenderTarget.h"

RenderTarget::RenderTarget(ID3D12Device* _device, HiddenDescriptorHeap* _heap, UINT _width, UINT _height, DXGI_FORMAT _format)
	:BaseResource()
{
	handle = _heap->GetNewDescriptorHandle(1);
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = _format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Buffer = { 0, _width * _height };
	desc.Texture2D = { 0, 0 };
	_device->CreateRenderTargetView(resource, &desc, handle.cpuHandle);
}
