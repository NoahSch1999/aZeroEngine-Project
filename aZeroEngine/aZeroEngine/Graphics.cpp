//#include "Graphics.h"
//#include <array>
//
//Graphics::Graphics(AppWindow& _window, HINSTANCE _instance)
//	:frameCount(0), frameIndex(0), ecs(1000), window(_window), 
//	materialManager(resourceEngine, textureCache), vbCache(resourceEngine), textureCache(resourceEngine)
//{
//	Initialize(_window, _instance);
//}
//
//Graphics::~Graphics()
//{
//	if(scene)
//		delete scene;
//
//	resourceEngine.RemoveResource(camera->GetBuffer());
//	vbCache.ShutDown();
//	textureCache.ShutDown();
//	materialManager.ShutDown();
//
//	/*swapChain->swapChain->Release();
//	swapChain->dxgiFactory->Release();*/
//	//resourceEngine.ShutDown();
//
//	device->Release();
//}
//
//void Graphics::Initialize(AppWindow& _window, HINSTANCE _instance)
//{
//	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
//	if (FAILED(hr))
//		throw;
//
//#ifdef _DEBUG
//	device->SetName(L"Main Device");
//#endif // DEBUG
//
//	swapChain = std::make_shared<SwapChain>();
//
//	resourceEngine.Init(device);
//	descriptorManager.Init(device, 100, 1000);
//	textureCache.Init();
//	vbCache.Init();
//	materialManager.Init();
//	//lManager.Init(device);
//
//	resourceEngine.Execute();
//
//	//swapChain = std::make_shared<SwapChain>(device, resourceEngine, descriptorManager, _window.GetHandle(), _window.GetWindowSize().x, window.GetWindowSize().y,
//	//	DXGI_FORMAT_B8G8R8A8_UNORM);
//
//	renderSystem = ecs.RegisterSystem<RendererSystem>();
//	lightSystem = ecs.RegisterSystem<LightSystem>();
//	pickingSystem = ecs.RegisterSystem<PickingSystem>();
//
//	//lightSystem->Init(device, &resourceEngine, &frameIndex);
//
//	renderSystem->Init(device,
//		&resourceEngine,
//		&vbCache,
//		lightSystem->GetLightManager(),
//		&materialManager,
//		swapChain.get(), _instance, window.GetHandle());
//
//	pickingSystem->Init(device, &resourceEngine, &vbCache, swapChain.get());
//
//	//camera = std::make_shared<Camera>(device, resourceEngine, _instance, _window.GetHandle(),
//	//	3.14f * 0.2f, (float)swapChain->width / (float)swapChain->height, 0.1f, 1000.f);
//
//	pickingSystem->SetCamera(camera);
//	renderSystem->SetMainCameraGeo(camera);
//
//	resourceEngine.Execute();
//
//	scene = nullptr;
//}
//
//void Graphics::BeginFrame()
//{
//	ImGui_ImplDX12_NewFrame();
//	ImGui_ImplWin32_NewFrame();
//	ImGui::NewFrame();
//	ImGuizmo::BeginFrame();
//
//	//frameIndex = frameCount % swapChain->numBackBuffers;
//	//currentBackBuffer = swapChain->backBuffers[frameIndex].get();
//	//renderSystem->SetBackBuffer(currentBackBuffer);
//
//	//ID3D12DescriptorHeap* heap[] = { descriptorManager.GetResourceHeap(), descriptorManager.GetSamplerHeap() };
//	//resourceEngine.renderPassList.GetGraphicList()->SetDescriptorHeaps(2, heap);
//
//	//D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetGPUOnlyResource().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
//	//resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);
//	//resourceEngine.renderPassList.GetGraphicList()->ClearRenderTargetView(currentBackBuffer->GetHandle().GetCPUHandle(), clearColor, 0, nullptr);
//	//resourceEngine.renderPassList.GetGraphicList()->ClearDepthStencilView(swapChain->dsv.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
//
//	for (auto& [name, ui] : userInterfaces)
//	{
//		ui->BeginFrame();
//	}
//
//}
//
//void Graphics::Render(AppWindow* _window)
//{
//	pickingSystem->Update();
//	renderSystem->Update();
//	lightSystem->Update();
//
//	for (auto& [name, ui] : userInterfaces)
//	{
//		ui->Update();
//	}
//}
//
//void Graphics::EndFrame()
//{
//
//	ImGui::Render();
//	//resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &swapChain->dsv.GetHandle().GetCPUHandleRef());
//	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), resourceEngine.renderPassList.GetGraphicList());
//
//	D3D12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetGPUOnlyResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
//	resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &x);
//
//	resourceEngine.Execute();
//
////	swapChain->swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
//
//	frameCount++;
//}