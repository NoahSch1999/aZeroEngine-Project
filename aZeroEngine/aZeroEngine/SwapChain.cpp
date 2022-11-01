#include "SwapChain.h"
#include "HelperFunctions.h"

SwapChain::SwapChain(ID3D12Device* _device, CommandQueue* _cmdQueue, CommandList* _cmdList, HiddenDescriptorHeap* _dsvHeap, HiddenDescriptorHeap* _heap, 
	HWND _winHandle, UINT _width, UINT _height, int _numBackBuffers, DXGI_FORMAT _rtvFormat, DXGI_FORMAT _dsvFormat)
{
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		throw;

	DXGI_SWAP_CHAIN_DESC1 scDesc;
	scDesc.SampleDesc.Quality = 0;
	scDesc.SampleDesc.Count = 1;
	scDesc.Format = _rtvFormat; // RGBA?
	scDesc.BufferCount = _numBackBuffers;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Width = _width;
	scDesc.Height = _height;
	scDesc.Stereo = false;

	DEVMODEA devModeA;
	devModeA.dmSize = sizeof(DEVMODE);
	bool x = Helper::GetDisplaySettings(&devModeA);
	refreshRate = devModeA.dmDisplayFrequency;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
	fullScreenDesc.RefreshRate.Numerator = refreshRate;
	fullScreenDesc.RefreshRate.Denominator = 1;
	fullScreenDesc.Windowed = false;
	fullScreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	hr = dxgiFactory->CreateSwapChainForHwnd(_cmdQueue->queue, _winHandle, &scDesc, nullptr, 0, &swapChain);
	if (FAILED(hr))
		throw;

	// create render targets and main depth buffer
	backBuffers.resize(_numBackBuffers);

	for (int i = 0; i < _numBackBuffers; i++)
	{
		backBuffers[i] = new RenderTarget();
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]->resource));
		if (FAILED(hr))
			throw;

		backBuffers[i]->handle = _heap->GetNewSlot();
		_device->CreateRenderTargetView(backBuffers[i]->resource, NULL, backBuffers[i]->handle.cpuHandle);
		std::wstring name = L"Back Buffer " + i;
		backBuffers[i]->resource->SetName(name.c_str());
	}
	rtvFormat = _rtvFormat;
	dsvFormat = _dsvFormat;
	numBackBuffers = _numBackBuffers;

	// Viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)_width;
	viewport.Height = (FLOAT)_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// Scissor Rect
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = _width;
	scissorRect.bottom = _height;

	dsv = new DepthStencil(_device, _dsvHeap, _cmdList, _width, _height, dsvFormat);
	dsv->resource->SetName(L"SwapChain DSV");
}

SwapChain::~SwapChain()
{
	swapChain->Release();
	dxgiFactory->Release();
	delete dsv;
}

void SwapChain::OnResize(HWND _winHandle)
{
	queue->queue->Signal(queue->fence, queue->nextFenceValue);
	queue->Flush(queue->nextFenceValue);
	queue->nextFenceValue++;

	UINT width, height;
	Helper::GetWindowDimensions(width, height); // doesnt set the appwindow variables...
	viewport.Width = (FLOAT)width;
	viewport.Height = (FLOAT)height;
	scissorRect.right = width;
	scissorRect.bottom = height;

	DXGI_MODE_DESC desc = {};
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.RefreshRate.Numerator = refreshRate;
	desc.RefreshRate.Denominator = 1;
	desc.Format = rtvFormat;
	desc.Scaling = DXGI_MODE_SCALING_STRETCHED;

	for (int i = 0; i < numBackBuffers; ++i)
	{
		backBuffers[i]->resource->Release();
	}

	HRESULT hr = swapChain->ResizeBuffers(numBackBuffers, width, height, rtvFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(hr))
		throw;

	for (int i = 0; i < numBackBuffers; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]->resource));
		device->CreateRenderTargetView(backBuffers[i]->resource, NULL, backBuffers[i]->handle.cpuHandle);
		backBuffers[i]->state = D3D12_RESOURCE_STATE_COMMON;
	}

	// Resize depth stencil etc...
	//dsv->ReInit(device, cmdList, width, height, dsvFormat);

	int next = queue->Execute(cmdList);
	queue->Flush(next);
}
