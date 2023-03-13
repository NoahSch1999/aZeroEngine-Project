#pragma once
#include <vector>
#include "ResourceEngine.h"

class SwapChain
{
private:

public:
	int numBackBuffers = 0;
	std::vector<RenderTarget*> backBuffers;
	DepthStencil dsv;

	IDXGISwapChain1* swapChain;
	IDXGIFactory2* dxgiFactory;
	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};

	int refreshRate = 0;
	SwapChain() = default;
	~SwapChain();

	void Init(ID3D12Device* _device, HWND _windowHandle, ResourceEngine& _resourceEngine, const Vector2& _clientDimensions, DescriptorHandle _dsvHandle, std::vector<DescriptorHandle> _bbHandles, int _numBackBuffers, DXGI_FORMAT _rtvFormat);

	void OnResize(HWND _winHandle/*, UINT& _width, UINT& _height*/);
};