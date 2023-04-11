#pragma once
#include <vector>
#include "ResourceEngine.h"
#include "DescriptorManager.h"
#include <array>

/** @brief Encapsulates everything related to the IDXGISwapChain interface.*/
class SwapChain
{
private:
	int numBackBuffers;
	std::array<std::unique_ptr<RenderTarget>, 3> backBuffers;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;

	D3D12_VIEWPORT viewport = {};
	D3D12_RECT scissorRect = {};

	DXGI_FORMAT bbFormat;
	DXGI_FORMAT dsvFormat;
	int width;
	int height;

	int refreshRate = 0;

public:
	
	SwapChain() = default;

	SwapChain(ID3D12Device* _device, ResourceEngine& _resourceEngine, HWND _windowHandle, int _width, int _height, DXGI_FORMAT _bbFormat,
		DXGI_SWAP_CHAIN_FLAG _flags = (DXGI_SWAP_CHAIN_FLAG)(DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING),
		DXGI_SCALING _scaling = DXGI_SCALING_STRETCH);

	~SwapChain() = default;

	int GetRefreshRate() const { return refreshRate; }
	int GetNumBackBuffers() const { return numBackBuffers; }
	Vector2 GetBackBufferDimensions() const { return { static_cast<float>(width), static_cast<float>(height) }; }

	D3D12_VIEWPORT& GetViewPort() { return viewport; }
	D3D12_RECT& GetScissorRect() { return scissorRect; }
	DXGI_FORMAT GetBackBufferFormat() const { return bbFormat; }

	RenderTarget* GetBackBuffer(int _index) const { return backBuffers[_index].get(); }
	std::array<std::unique_ptr<RenderTarget>, 3>& GetBackBuffers() { return backBuffers; }
	IDXGISwapChain1* GetSwapChain() const { return swapChain.Get(); }

};