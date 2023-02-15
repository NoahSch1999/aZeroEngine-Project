#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow& _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0), ecs(1000), window(_window)
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	WaitForGPU();

	device->Release();

	delete rtvHeap;
	delete dsvHeap;

	delete scene;
}

void Graphics::Initialize(AppWindow& _window, HINSTANCE _instance)
{
	// Device
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
	if (FAILED(hr))
		throw;

	// Direct / Main command queue
	directCommandQueue.Init(device, 
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, 
		D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE);

	rtvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, 1, L"RTV Heap");
	dsvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, 1, L"DSV Heap");

	allocator.Init(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	directCmdList.Init(device, allocator);

	resourceManager.Init(device, 100, 1500);

	textureCache.Init(device, directCmdList, resourceManager);

	materialManager.Init(device, directCmdList, &resourceManager, &textureCache);

	WaitForGPU();

	lManager.Init(device, directCmdList, 1, 10, 1);

	scene = nullptr;

	PointLight l;
	l.color = { 1,1,0 };
	l.position = { 0,2, 0 };
	l.strenght = 40;
	int id = -1;
	lManager.AddLight(l, id);

	l.color = { 1,0,1 };
	l.position = { 1,0,0 };
	lManager.AddLight(l, id);

	l.color = { 1,1,0 };
	l.position = { 1,1,0 };
	lManager.AddLight(l, id);

	l.color = { 1,0,0.5f };
	l.position = { 1,1,1 };
	lManager.AddLight(l, id);

	WaitForGPU();

	vbCache.ReleaseIntermediateResources();
	textureCache.ReleaseIntermediateResources();
}

void Graphics::Begin()
{
	frameIndex = frameCount % window.GetSwapChain().numBackBuffers;
	currentBackBuffer = window.GetSwapChain().backBuffers[frameIndex];
	currentBackBuffer->TransitionMain(directCmdList.GetGraphicList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	directCmdList.GetGraphicList()->RSSetViewports(1, &window.GetSwapChain().viewport);
	directCmdList.GetGraphicList()->RSSetScissorRects(1, &window.GetSwapChain().scissorRect);
	directCmdList.GetGraphicList()->ClearRenderTargetView(currentBackBuffer->GetHandle().GetCPUHandle(), clearColor, 0, nullptr);
	directCmdList.GetGraphicList()->ClearDepthStencilView(window.GetSwapChain().dsv.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	directCmdList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &window.GetSwapChain().dsv.GetHandle().GetCPUHandleRef());
}

void Graphics::Render(AppWindow* _window)
{
	ID3D12DescriptorHeap* heap[] = { resourceManager.GetResourceHeap(), resourceManager.GetSamplerHeap() };
	directCmdList.GetGraphicList()->SetDescriptorHeaps(2, heap);

	renderSystem->Update();
}

void Graphics::Present()
{
	currentBackBuffer->TransitionMain(directCmdList.GetGraphicList(), D3D12_RESOURCE_STATE_PRESENT);
	nextSyncSignal = directCommandQueue.Execute(directCmdList);

	// might enqueue work on gpu...
	window.GetSwapChain().swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	
	if (frameCount % 3 == 0)
	{
		directCommandQueue.Flush(nextSyncSignal);
		allocator.Reset();
	}

	directCmdList.ResetGraphic(allocator);

	frameCount++;
}

void Graphics::WaitForGPU()
{
	int nextSignal = directCommandQueue.Execute(directCmdList);
	directCommandQueue.Flush(nextSignal, allocator, directCmdList);
}
