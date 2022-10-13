#pragma once
#include "D3D12Include.h"
#include "CommandQueue.h"
#include <vector>
#include "RenderTarget.h"
#include "HiddenDescriptorHeap.h"
//#include "AppWindow.h"
#include "DepthStencil.h"

class SwapChain
{
public:
	DXGI_FORMAT rtvFormat;
	DXGI_FORMAT dsvFormat;
	int numBackBuffers;
	std::vector<RenderTarget*> backBuffers;
	DepthStencil* dsv;

	IDXGISwapChain1* swapChain;
	IDXGIFactory2* dxgiFactory;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;

	int refreshRate;

	// render target and dsv
	SwapChain(ID3D12Device* _device, CommandQueue* _cmdQueue, CommandList* _cmdList, HiddenDescriptorHeap* _dsvHeap, HiddenDescriptorHeap* _heap, 
		HWND _winHandle, UINT _width, UINT _height, int _numBackBuffers, DXGI_FORMAT _rtvFormat, DXGI_FORMAT _dsvFormat);
	~SwapChain();

	//void SetFullscreen(AppWindow* _window);
	//void SetWindowed(AppWindow* _window, int width, int height);

	void OnResize(HWND _winHandle/*, UINT& _width, UINT& _height*/);
};