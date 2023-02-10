#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow* _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0), ecs(1000)
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	WaitForGPU();

	delete swapChain;

	device->Release();

	delete rtvHeap;
	delete dsvHeap;

	delete scene;
}

void Graphics::Initialize(AppWindow* _window, HINSTANCE _instance)
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

	materialManager.Init(&resourceManager);

	swapChain = new SwapChain(device, &directCommandQueue, &directCmdList, dsvHeap, rtvHeap, _window->windowHandle, _window->width, _window->height, 3, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);

	WaitForGPU();

	swapChain->queue = &directCommandQueue;
	swapChain->syncValue = &nextSyncSignal;
	swapChain->device = device;
	swapChain->cmdList = &directCmdList;

	//bool debug = true;

	//if (debug)
	//{
	//	vbCache.LoadResource(device, &directCmdList, "goblin");

	//	textureCache.LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "defaultDiffuse.png");
	//	textureCache.LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "goblintexture.png");

	//	materialManager.CreateMaterial<PhongMaterial>(device, directCmdList, textureCache, "defaultMaterial");

	//	lManager.Init(device, directCmdList, 1, 10, 1);

	//	scene = new Scene(ecs, vbCache, materialManager, resourceManager, textureCache);

	//	for (int i = 0; i < 5; i++)
	//	{
	//		float xPos = i;
	//		float zPos = 0;
	//		if (i % 2 == 0)
	//		{
	//			xPos /= 2.f;
	//			zPos = i;
	//		}
	//		Entity& tempEnt = scene->CreateEntity(device, &directCmdList);

	//		Mesh mesh;
	//		mesh.vbIndex = vbCache.GetBufferIndex("goblin");
	//		scene->AddComponentToEntity<Mesh>(tempEnt, mesh);

	//		MaterialComponent mat;
	//		mat.materialID = materialManager.GetReferenceID<PhongMaterial>("defaultMaterial");
	//		scene->AddComponentToEntity<MaterialComponent>(tempEnt, mat);

	//		ecs.GetComponentManager().GetComponent<Transform>(tempEnt)->cb.InitAsCBV(device, resourceManager.GetPassDescriptor());
	//		Matrix x = Matrix::CreateTranslation(xPos, 0, zPos);
	//		ecs.GetComponentManager().GetComponent<Transform>(tempEnt)->Update(&directCmdList, x, 0);
	//	}
	//}
	//else
	//{

	lManager.Init(device, directCmdList, 1, 10, 1);

	scene = new Scene(ecs, vbCache, materialManager, resourceManager, textureCache);
	//}

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

	renderSystem = new BasicRendererSystem(device, directCmdList, ecs, materialManager, resourceManager, lManager, vbCache, *swapChain, _window, _instance);

	for (const auto& ent : scene->entities.GetObjects())
	{
		renderSystem->Bind(ent);
	}
}

void Graphics::Begin()
{
	frameIndex = frameCount % swapChain->numBackBuffers;
	currentBackBuffer = swapChain->backBuffers[frameIndex];
	currentBackBuffer->TransitionMain(directCmdList.GetGraphicList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	directCmdList.GetGraphicList()->RSSetViewports(1, &swapChain->viewport);
	directCmdList.GetGraphicList()->RSSetScissorRects(1, &swapChain->scissorRect);
	directCmdList.GetGraphicList()->ClearRenderTargetView(currentBackBuffer->GetHandle().GetCPUHandle(), clearColor, 0, nullptr);
	directCmdList.GetGraphicList()->ClearDepthStencilView(swapChain->dsv->GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	directCmdList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &swapChain->dsv->GetHandle().GetCPUHandleRef());
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
	swapChain->swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	
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
