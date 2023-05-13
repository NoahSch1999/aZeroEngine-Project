#include "SwapChain.h"
#include "HelperFunctions.h"

SwapChain::SwapChain(ID3D12Device* device, CommandQueue& commandQueue, DescriptorManager& descriptorManager, ResourceTrashcan& trashCan,
	HWND windowHandle, int width, int height, DXGI_FORMAT bbFormat, DXGI_SWAP_CHAIN_FLAG flags, DXGI_SCALING scaling)
	:m_numBackBuffers(3), m_bbFormat(bbFormat), m_dsvFormat(DXGI_FORMAT_D24_UNORM_S8_UINT), m_width(width), m_height(height)
{
	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&m_dxgiFactory))))
		throw;

	DXGI_SWAP_CHAIN_DESC1 scDesc;
	scDesc.SampleDesc.Quality = 0;
	scDesc.SampleDesc.Count = 1;
	scDesc.Format = bbFormat;
	scDesc.BufferCount = m_numBackBuffers;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Scaling = DXGI_SCALING_STRETCH;
	scDesc.Flags = flags;
	scDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scDesc.Width = width;
	scDesc.Height = height;
	scDesc.Stereo = false;

	DEVMODEA devModeA;
	devModeA.dmSize = sizeof(DEVMODE);
	bool x = Helper::GetDisplaySettings(&devModeA);
	m_refreshRate = devModeA.dmDisplayFrequency;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
	fullScreenDesc.RefreshRate.Numerator = m_refreshRate;
	fullScreenDesc.RefreshRate.Denominator = 1;
	fullScreenDesc.Windowed = false;
	fullScreenDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	if(FAILED(m_dxgiFactory->CreateSwapChainForHwnd(commandQueue.getQueue(), windowHandle, &scDesc, nullptr, 0, &m_swapChain)))
		throw;

	std::vector<DescriptorHandle> bbHandles = descriptorManager.getRTVDescriptor(m_numBackBuffers);
	for (int i = 0; i < m_numBackBuffers; i++)
	{
		m_backBuffers[i] = std::make_unique<Texture>();
		m_backBuffers[i]->m_trashcan = &trashCan;
		m_backBuffers[i]->m_descriptorManager = &descriptorManager;

		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]->m_gpuOnlyResource));

		m_backBuffers[i]->m_handleRTVDSV = bbHandles[i];
		m_backBuffers[i]->setResourceState(D3D12_RESOURCE_STATE_COMMON);
		device->CreateRenderTargetView(m_backBuffers[i]->m_gpuOnlyResource.Get(), NULL, m_backBuffers[i]->m_handleRTVDSV.getCPUHandle());

#ifdef _DEBUG
		const std::string name("Back Buffer " + std::to_string(i));
		const std::wstring wName(name.begin(), name.end());
		m_backBuffers[i]->m_gpuOnlyResource->SetName(wName.c_str());
#endif // DEBUG
	}

	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = static_cast<LONG>(width);
	m_scissorRect.bottom = static_cast<LONG>(height);
}