#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow& _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0), ecs(1000), window(_window), 
	lManager(resourceEngine), materialManager(resourceEngine, descriptorManager, textureCache), vbCache(resourceEngine), textureCache(resourceEngine, descriptorManager)
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	vbCache.ShutDown();
	textureCache.ShutDown();
	lManager.ShutDown();
	materialManager.ShutDown();
	delete shadowSystem;
	delete renderSystem;

	resourceEngine.ShutDown();

	device->Release();
}

void Graphics::Initialize(AppWindow& _window, HINSTANCE _instance)
{
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
	if (FAILED(hr))
		throw;

	resourceEngine.Init(device);
	descriptorManager.Init(device, 100, 1000);
	textureCache.Init(device);
	materialManager.Init(device);
	lManager.Init(device);
	vbCache.LoadResource(device, "demoCube", "..\\meshes\\");

	resourceEngine.Execute(frameIndex);

	scene = nullptr;
}

void Graphics::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	frameIndex = frameCount % window.GetSwapChain().numBackBuffers;
	currentBackBuffer = window.GetSwapChain().backBuffers[frameIndex];
	renderSystem->currentBackBuffer = currentBackBuffer;
	renderSystem->dsv = &window.GetSwapChain().dsv;

	ID3D12DescriptorHeap * heap[] = { descriptorManager.GetResourceHeap(), descriptorManager.GetSamplerHeap() };
	resourceEngine.renderPassList.GetGraphicList()->SetDescriptorHeaps(2, heap);
	resourceEngine.renderPassList.GetGraphicList()->RSSetViewports(1, &window.GetSwapChain().viewport);
	resourceEngine.renderPassList.GetGraphicList()->RSSetScissorRects(1, &window.GetSwapChain().scissorRect);

	D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetMainResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);
	resourceEngine.renderPassList.GetGraphicList()->ClearRenderTargetView(currentBackBuffer->GetHandle().GetCPUHandle(), clearColor, 0, nullptr);
	resourceEngine.renderPassList.GetGraphicList()->ClearDepthStencilView(window.GetSwapChain().dsv.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

	for (auto& [name, ui] : userInterfaces)
	{
		ui->BeginFrame();
	}

}

void Graphics::Render(AppWindow* _window)
{
	
	// shadow pass
	shadowSystem->Update();

	// geometry pass
	renderSystem->Update();

	for (auto& [name, ui] : userInterfaces)
	{
		ui->Update();
	}

	shadowSystem->End();
}

void Graphics::EndFrame()
{

	ImGui::Render();
	resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &window.GetSwapChain().dsv.GetHandle().GetCPUHandleRef());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), resourceEngine.renderPassList.GetGraphicList());

	D3D12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetMainResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &x);

	resourceEngine.Execute(frameIndex);

	window.GetSwapChain().swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);

	frameCount++;
}