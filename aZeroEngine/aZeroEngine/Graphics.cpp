#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow* _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0)
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	WaitForGPU();
	/*allocator->allocator->Reset();
	directCmdList.graphic->Reset(allocator->allocator, nullptr);*/

	delete directCommandQueue;
	delete swapChain;
	delete rasterState;
	device->Release();
	delete allocator;

	delete rtvHeap;
	delete dsvHeap;

	camera->diMouseDevice->Release();
	camera->buffer->GetResource()->Release();
	camera->buffer->uploadBuffer->Release();
	delete camera->buffer;

	delete ui;
	delete sampler;
	delete vbCache;

	delete textureCache;
	delete lManager;
	delete scene;
	delete ecs;
}

void Graphics::Initialize(AppWindow* _window, HINSTANCE _instance)
{
	// Device
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
	if (FAILED(hr))
		throw;

	// Direct / Main command queue
	directCommandQueue = new CommandQueue(device, 
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, 
		D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE);

	rtvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, 1, L"RTV Heap");
	dsvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, 1, L"DSV Heap");
	allocator = new CommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	directCmdList.Init(device, allocator);

	resourceManager.Init(device, 100, 1500);

	swapChain = new SwapChain(device, directCommandQueue, &directCmdList, dsvHeap, rtvHeap, _window->windowHandle, _window->width, _window->height, 3, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);

	rasterState = new RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);

	sampler = new Sampler(device, resourceManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

	RootParameters params;
	params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 100, D3D12_SHADER_VISIBILITY_ALL, 0, 1);								// textures 0
	params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// perdrawconstants 1
	params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);			// world matrix 2
	//params.AddRootConstants(0, 16, D3D12_SHADER_VISIBILITY_VERTEX, 0);																	// world matrix 2
	params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);			// camera 3
	params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// sampler 4
	params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// point light structs 5
	params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// point light indices 6
	params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);			// num lights
	bindlessSignature.Initialize(device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

	pso.Init(device, &bindlessSignature, layout, rasterState, swapChain->numBackBuffers, swapChain->rtvFormat, swapChain->dsvFormat,
		L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Basic.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Basic.cso",
		L"", L"", L"");

	int nextSignal = directCommandQueue->Execute(&directCmdList);
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);

	//
	camera = new Camera(device, &directCmdList, _window->width, _window->height, _instance, _window->windowHandle);

	swapChain->queue = directCommandQueue;
	swapChain->syncValue = &nextSyncSignal;
	swapChain->device = device;
	swapChain->cmdList = &directCmdList;

	vbCache = new VertexBufferCache();
	textureCache = new Texture2DCache();

	bool debug = true;

	if (debug)
	{
		vbCache->LoadBuffer(device, &directCmdList, "goblin");

		textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "defaultDiffuse.png");
		textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "goblintexture.png");

		materialManager.CreateMaterial<PhongMaterial>(device, &directCmdList, textureCache, "defaultMaterial");
		materialManager.CreateMaterial<PhongMaterial>(device, &directCmdList, textureCache, "otherMaterial");
		materialManager.GetMaterial<PhongMaterial>("otherMaterial")->GetInfoPtr()->diffuseTextureID = textureCache->GetResource("goblintexture.png").GetHandle().GetHeapIndex();
		materialManager.GetMaterial<PhongMaterial>("otherMaterial")->Update(&directCmdList, frameIndex);

		ui = new EditorUI(device, &resourceManager, _window->windowHandle);

		lManager = new LightManager(device, &directCmdList, 1, 10, 1);

		ecs = new ECS(100);
		scene = new Scene(ecs, vbCache, &materialManager, &resourceManager, textureCache);

		for (int i = 0; i < 5; i++)
		{
			float xPos = i;
			float zPos = 0;
			if (i % 2 == 0)
			{
				xPos /= 2.f;
				zPos = i;
			}
			Entity& tempEnt = scene->CreateEntity(device, &directCmdList);

			Mesh mesh;
			mesh.vbIndex = vbCache->GetBufferIndex("goblin");
			scene->AddComponentToEntity<Mesh>(tempEnt, mesh);

			MaterialComponent mat;
			mat.materialID = materialManager.GetReferenceID<PhongMaterial>("defaultMaterial");
			scene->AddComponentToEntity<MaterialComponent>(tempEnt, mat);

			ecs->GetComponentManager().GetComponent<Transform>(tempEnt)->cb.InitAsCBV(device, resourceManager.GetPassDescriptor());
			Matrix x = Matrix::CreateTranslation(xPos, 0, zPos);
			ecs->GetComponentManager().GetComponent<Transform>(tempEnt)->Update(&directCmdList, x, 0);
		}
	}
	else
	{
		ui = new EditorUI(device, &resourceManager, _window->windowHandle);

		lManager = new LightManager(device, &directCmdList, 1, 10, 1);

		ecs = new ECS(100);
		scene = new Scene(ecs, vbCache, &materialManager, &resourceManager, textureCache);

		//scene->Load(device, &directCmdList, frameIndex, "C:/Users/Noah Schierenbeck/Desktop/Test", "Level1");
	}

	nextSignal = directCommandQueue->Execute(&directCmdList);
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);
}

void Graphics::Begin()
{
	frameIndex = frameCount % swapChain->numBackBuffers;
	currentBackBuffer = swapChain->backBuffers[frameIndex];
	
	currentBackBuffer->Transition(directCmdList.graphic, D3D12_RESOURCE_STATE_RENDER_TARGET);
	directCmdList.graphic->RSSetViewports(1, &swapChain->viewport);
	directCmdList.graphic->RSSetScissorRects(1, &swapChain->scissorRect);
	FLOAT x[4] = { 1,1,0,0 };
	directCmdList.graphic->OMSetBlendFactor(x);
	directCmdList.graphic->ClearRenderTargetView(currentBackBuffer->GetHandle().GetCPUHandle(), clearColor, 0, nullptr);
	directCmdList.graphic->ClearDepthStencilView(swapChain->dsv->GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	directCmdList.graphic->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &swapChain->dsv->GetHandle().GetCPUHandleRef());
}

void Graphics::Render(AppWindow* _window)
{
	ID3D12DescriptorHeap* heap[] = { resourceManager.GetResourceHeap(), resourceManager.GetSamplerHeap() };
	directCmdList.graphic->SetDescriptorHeaps(2, heap);
	directCmdList.graphic->SetPipelineState(pso.GetPipelineState());
	directCmdList.graphic->SetGraphicsRootSignature(bindlessSignature.signature);
	directCmdList.graphic->SetGraphicsRootDescriptorTable(0, resourceManager.GetTexture2DStartAddress());
	directCmdList.graphic->SetGraphicsRootConstantBufferView(3, camera->buffer->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootShaderResourceView(5, lManager->pLightList.GetLightsBufferPtr()->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootShaderResourceView(6, lManager->pLightList.GetLightsIndicesBufferPtr()->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootConstantBufferView(7, lManager->numLightsCB.GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootDescriptorTable(4, sampler->GetHandle().GetGPUHandle());
	directCmdList.graphic->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ComponentManager& cManager = ecs->GetComponentManager();
	for (const Entity& ent : scene->entities.GetObjects())
	{
		directCmdList.graphic->IASetVertexBuffers(0, 1, &vbCache->GetBuffer(cManager.GetComponent<Mesh>(ent)->vbIndex)->GetView());
		directCmdList.graphic->SetGraphicsRootConstantBufferView(2, cManager.GetComponent<Transform>(ent)->cb.GetGPUAddress());
		directCmdList.graphic->SetGraphicsRootConstantBufferView(1, materialManager.GetMaterial<PhongMaterial>(cManager.GetComponent<MaterialComponent>(ent)->materialID)->GetGPUAddress());
		directCmdList.graphic->DrawInstanced(vbCache->GetBuffer(cManager.GetComponent<Mesh>(ent)->vbIndex)->GetNumVertices(), 1, 0, 0);
	}

	ui->Update();
	ui->Render(&directCmdList);
}

void Graphics::Present()
{
	currentBackBuffer->Transition(directCmdList.graphic, D3D12_RESOURCE_STATE_PRESENT);
	nextSyncSignal = directCommandQueue->Execute(&directCmdList);

	// might enqueue work on gpu...
	swapChain->swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	
	if (frameCount % 3 == 0)
	{
		directCommandQueue->Flush(nextSyncSignal);
		allocator->allocator->Reset();
	}

	directCmdList.graphic->Reset(allocator->allocator, nullptr);

	frameCount++;
}

void Graphics::WaitForGPU()
{
	int nextSignal = directCommandQueue->Execute(&directCmdList);
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);
}
