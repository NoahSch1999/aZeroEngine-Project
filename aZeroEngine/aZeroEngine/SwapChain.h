#pragma once
#include "CommandQueue.h"
#include <vector>
#include "RenderTarget.h"
#include "DepthStencil.h"

class SwapChain
{
private:

public:
	int numBackBuffers;
	std::vector<RenderTarget*> backBuffers;
	DepthStencil dsv;

	IDXGISwapChain1* swapChain;
	IDXGIFactory2* dxgiFactory;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	int refreshRate;
	SwapChain() = default;
	~SwapChain();

	void Init(ID3D12Device* _device, HWND _windowHandle, CommandQueue& _cmdQueue, CommandList& _cmdList, const Vector2& _clientDimensions, DescriptorHandle _dsvHandle, std::vector<DescriptorHandle> _bbHandles, int _numBackBuffers, DXGI_FORMAT _rtvFormat, DXGI_FORMAT _dsvFormat);

	void OnResize(HWND _winHandle/*, UINT& _width, UINT& _height*/);
};