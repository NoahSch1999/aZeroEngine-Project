#pragma once
#include <vector>
#include "CommandQueue.h"
#include "DescriptorManager.h"
#include <array>
#include "Texture.h"

/** @brief Encapsulates everything related to the IDXGISwapChain interface.*/
class SwapChain
{
private:
	int m_numBackBuffers;
	std::array<std::unique_ptr<Texture>, 3> m_backBuffers;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgiFactory;

	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_scissorRect = {};

	DXGI_FORMAT m_bbFormat;
	DXGI_FORMAT m_dsvFormat;
	int m_width;
	int m_height;

	int m_refreshRate = 0;

public:
	
	SwapChain() = default;

	SwapChain(ID3D12Device* device, CommandQueue& commandQueue, DescriptorManager& descriptorManager, ResourceRecycler& trashCan, HWND windowHandle, int width, int height, DXGI_FORMAT bbFormat,
		DXGI_SWAP_CHAIN_FLAG flags = (DXGI_SWAP_CHAIN_FLAG)(DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING),
		DXGI_SCALING scaling = DXGI_SCALING_STRETCH);

	~SwapChain() = default;

	int getRefreshRate() const { return m_refreshRate; }
	int getNumBackBuffers() const { return m_numBackBuffers; }
	DXM::Vector2 getBackBufferDimensions() const { return { static_cast<float>(m_width), static_cast<float>(m_height) }; }

	D3D12_VIEWPORT& getViewPort() { return m_viewport; }
	D3D12_RECT& getScissorRect() { return m_scissorRect; }
	DXGI_FORMAT getBackBufferFormat() const { return m_bbFormat; }

	Texture* getBackBuffer(int index) const { return m_backBuffers[index].get(); }
	std::array<std::unique_ptr<Texture>, 3>& getBackBuffers() { return m_backBuffers; }
	IDXGISwapChain1* getSwapChain() const { return m_swapChain.Get(); }

	void resizeBackBuffers(ID3D12Device* device, UINT width, UINT height);
};