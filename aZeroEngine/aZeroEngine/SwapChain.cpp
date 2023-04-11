#include "SwapChain.h"
#include "HelperFunctions.h"

SwapChain::SwapChain(ID3D12Device* _device, ResourceEngine& _resourceEngine, HWND _windowHandle, int _width, int _height, DXGI_FORMAT _bbFormat, DXGI_SWAP_CHAIN_FLAG _flags, DXGI_SCALING _scaling)
	:numBackBuffers(3), bbFormat(_bbFormat), dsvFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), width(_width), height(_height)
{
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		throw;

	DXGI_SWAP_CHAIN_DESC1 scDesc;
	scDesc.SampleDesc.Quality = 0;
	scDesc.SampleDesc.Count = 1;
	scDesc.Format = _bbFormat;
	scDesc.BufferCount = numBackBuffers;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Flags = _flags;
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

	hr = dxgiFactory->CreateSwapChainForHwnd(_resourceEngine.GetDirectQueue(), _windowHandle, &scDesc, nullptr, 0, &swapChain);
	if (FAILED(hr))
		throw;

	std::vector<DescriptorHandle> bbHandles = _resourceEngine.GetDescriptorManager().GetRTVDescriptor(numBackBuffers);
	for (int i = 0; i < numBackBuffers; i++)
	{
		backBuffers[i] = std::make_unique<RenderTarget>();

		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]->GetGPUOnlyResource()));

		backBuffers[i]->GetHandle() = bbHandles[i];
		_device->CreateRenderTargetView(backBuffers[i]->GetGPUOnlyResource().Get(), NULL, backBuffers[i]->GetHandle().GetCPUHandle());

#ifdef _DEBUG
		const std::string name("Back Buffer " + std::to_string(i));
		const std::wstring wName(name.begin(), name.end());
		backBuffers[i]->GetGPUOnlyResource()->SetName(wName.c_str());
#endif // DEBUG
	}

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)_width;
	viewport.Height = (FLOAT)_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = _width;
	scissorRect.bottom = _height;

//	_resourceEngine.CreateResource(dsv, _width, _height, false);
//
//#ifdef _DEBUG
//	dsv.GetGPUOnlyResource()->SetName(L"Swap Chain Depth Stencil");
//#endif // DEBUG
}