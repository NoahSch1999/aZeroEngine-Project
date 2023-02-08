#include "DepthStencil.h"

DepthStencil::DepthStencil(ID3D12Device* _device, HiddenDescriptorHeap* _heap, CommandList* _cmdList, UINT _width, UINT _height, DXGI_FORMAT _format)
	:BaseResource()
{
	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = _width;
	rDesc.Height = _height;
	rDesc.DepthOrArraySize = 1; // ?
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES properties = {};
	properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = _format;
	clearValue.Color[0] = 1;
	clearValue.Color[1] = 1;
	clearValue.Color[2] = 1;
	clearValue.Color[3] = 1;
	clearValue.DepthStencil.Depth = 1;
	clearValue.DepthStencil.Stencil = 0;

	// how to avoid creating implicit descriptor heap? replace with placeresource... or atleast provide that option
	HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&mainResource));
	if (FAILED(hr))
		throw;

	handle = _heap->GetNewSlot();
	_device->CreateDepthStencilView(mainResource, nullptr, handle.GetCPUHandle());

	TransitionMain(_cmdList->graphic, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void DepthStencil::ReInit(ID3D12Device* _device, CommandList* _cmdList, UINT _width, UINT _height, DXGI_FORMAT _format)
{
	mainResource->Release();

	D3D12_RESOURCE_DESC rDesc = {};
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Width = _width;
	rDesc.Height = _height;
	rDesc.DepthOrArraySize = 1; // ?
	rDesc.MipLevels = 1;
	rDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES properties = {};
	properties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = _format;
	clearValue.Color[0] = 1;
	clearValue.Color[1] = 1;
	clearValue.Color[2] = 1;
	clearValue.Color[3] = 1;
	clearValue.DepthStencil.Depth = 1;
	clearValue.DepthStencil.Stencil = 0;

	// how to avoid creating implicit descriptor heap? replace with placeresource... or atleast provide that option
	HRESULT hr = _device->CreateCommittedResource(&properties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&rDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(&mainResource));
	if (FAILED(hr))
		throw;

	_device->CreateDepthStencilView(mainResource, nullptr, handle.GetCPUHandle());

	TransitionMain(_cmdList->graphic, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

DepthStencil::~DepthStencil()
{
}

void DepthStencil::InitStatic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, const std::wstring& _mainResourceName)
{
	throw; // Not implemented yet
}

void DepthStencil::InitDynamic(ID3D12Device* _device, CommandList* _cmdList, void* _initData, int _numBytes, int _numElements, bool _trippleBuffered, const std::wstring& _mainResourceName)
{
	throw; // Not implemented yet
}
