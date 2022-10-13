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

		backBuffers[i]->handle = _heap->GetNewDescriptorHandle(1);
		_device->CreateRenderTargetView(backBuffers[i]->resource, NULL, backBuffers[i]->handle.cpuHandle);
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

	//SetFullscreen(_window);

	dsv = new DepthStencil(_device, _dsvHeap, _cmdList, _width, _height, dsvFormat);
	dsv->resource->SetName(L"SwapChain DSV");
}

SwapChain::~SwapChain()
{
	swapChain->Release();
	dxgiFactory->Release();
	delete dsv;

	// Why not work....?
	//for (int i = 0; i < numBackBuffers; i++)
	//	delete backBuffers[i];
}

//void SwapChain::SetFullscreen(AppWindow* _window)
//{
//	Helper::GetWindowDimensions(_window);
//
//	DXGI_MODE_DESC desc = {};
//	ZeroMemory(&desc, sizeof(desc));
//	desc.Width = _window->width;
//	desc.Height = _window->height;
//	desc.RefreshRate.Numerator = refreshRate;
//	desc.RefreshRate.Denominator = 1;
//	desc.Format = rtvFormat;
//	desc.Scaling = DXGI_MODE_SCALING_STRETCHED;
//	//swapChain->ResizeTarget(&desc);
//
//	// Why doesn't it go into fullscreen? it did earlier, but something changed...
//	HRESULT hr = swapChain->SetFullscreenState(true, NULL);
//	if (FAILED(hr))
//		throw;
//
//	//SetWindowPos(_window->windowHandle, 0, 0, 0, _window->width, _window->height, SWP_SHOWWINDOW); // Needed?
//
//	viewport.Width = (FLOAT)_window->width;
//	viewport.Height = (FLOAT)_window->height;
//	scissorRect.right = _window->width;
//	scissorRect.bottom = _window->height;
//
//	// They need to be released and recreated... but in order to do this, they have to be unbound from the pipeline
//	//for (int i = 0; i < numBackBuffers; i++)
//	//	backBuffers[i]->resource->Release();
//
//	hr = swapChain->ResizeBuffers(numBackBuffers, _window->width, _window->height, rtvFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
//	if (FAILED(hr))
//		throw;
//	// DXGI ERROR: IDXGISwapChain::ResizeBuffers: Swapchain cannot be resized unless all outstanding buffer references have been released. [ MISCELLANEOUS ERROR #19: ]
//
//	// Resize textures such as depth stencils etc...
//
//}
//
//void SwapChain::SetWindowed(AppWindow* _window, int _width, int _height)
//{
//	_window->width = _width;
//	_window->height = _height;
//
//	DXGI_MODE_DESC desc = {};
//	ZeroMemory(&desc, sizeof(desc));
//	desc.Width = _window->width;
//	desc.Height = _window->height;
//	desc.RefreshRate.Numerator = refreshRate;
//	desc.RefreshRate.Denominator = 1;
//	desc.Format = rtvFormat;
//	desc.Scaling = DXGI_MODE_SCALING_STRETCHED;
//
//	swapChain->SetFullscreenState(false, NULL);
//
//	swapChain->ResizeTarget(&desc);
//
//	swapChain->ResizeBuffers(numBackBuffers, _window->width, _window->height, rtvFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
//
//	// Resize textures such as depth stencils etc...
//
//}

void SwapChain::OnResize(HWND _winHandle/*, UINT& _width, UINT& _height*/)
{
	// Resize buffers etc...
	UINT width, height;
	//_width = width;
	//_height = height;
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
	HRESULT hr = swapChain->ResizeTarget(&desc);
	if (FAILED(hr))
		throw;

	hr = swapChain->ResizeBuffers(numBackBuffers, width, height, rtvFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(hr))
		throw;
}
