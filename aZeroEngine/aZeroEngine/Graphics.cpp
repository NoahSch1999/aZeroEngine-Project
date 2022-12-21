#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow* _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0)
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	directCommandQueue->Flush(nextSyncSignal);
	allocator->allocator->Reset();
	directCmdList.graphic->Reset(allocator->allocator, nullptr);

	delete directCommandQueue;
	delete swapChain;
	delete rasterState;
	device->Release();
	delete rtvHeap, dsvHeap;
	delete allocator;
	delete camera->buffer;
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

	pso.Init(device, &bindlessSignature, &layout, rasterState, swapChain->numBackBuffers, swapChain->rtvFormat, swapChain->dsvFormat,
		L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Basic.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Basic.cso",
		L"", L"", L"");

	int nextSignal = directCommandQueue->Execute(&directCmdList);
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);

	//
	camera = new Camera(device, &directCmdList, _window->width, _window->height, _instance, _window->windowHandle);

	Matrix x = Matrix::Identity;
	x = Matrix::CreateTranslation(0, 0, 0);
	testWorld.InitAsDynamic(device, &directCmdList, (void*)&x, sizeof(Matrix), true, L"testWorld");
	x = Matrix::CreateTranslation(1, 0, 0);
	testWorld2.InitAsDynamic(device, &directCmdList, (void*)&x, sizeof(Matrix), true, L"testWorld2");

	swapChain->queue = directCommandQueue;
	swapChain->syncValue = &nextSyncSignal;
	swapChain->device = device;
	swapChain->cmdList = &directCmdList;

	vbCache = new VertexBufferCache();
	vbCache->LoadBuffer(device, &directCmdList, "cube");
	vbCache->LoadBuffer(device, &directCmdList, "sphere");
	vbCache->LoadBuffer(device, &directCmdList, "goblin");
	//vbCache->LoadBuffer(device, &directCmdList, "Wa");

	textureCache = new Texture2DCache();
	textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "pylot.png");
	textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "sadcat.png");
	textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "brickAlbedo.png");
	textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "defaultDiffuse.png");
	textureCache->LoadResource(device, resourceManager.GetTexture2DDescriptor(), &directCmdList, "goblintexture.png");

	materialManager.CreateMaterial<PhongMaterial>(device, &directCmdList, textureCache, "testMaterial");
	materialManager.CreateMaterial<PhongMaterial>(device, &directCmdList, textureCache, "otherMaterial");
	materialManager.GetMaterial<PhongMaterial>("otherMaterial")->GetInfoPtr()->diffuseTextureID = textureCache->GetResource("pylot.png")->handle.heapIndex;
	materialManager.GetMaterial<PhongMaterial>("otherMaterial")->Update(&directCmdList, 0);

	testIDMaterial = materialManager.GetReferenceID<PhongMaterial>("otherMaterial");

	testMeshID = vbCache->GetReferenceID("goblin");

	ui = new EditorUI(device, &resourceManager, _window->windowHandle);

	lManager = new LightManager(device, &directCmdList, 1, 10, 1);

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
	directCmdList.graphic->ClearRenderTargetView(currentBackBuffer->handle.cpuHandle, clearColor, 0, nullptr);
	directCmdList.graphic->ClearDepthStencilView(swapChain->dsv->handle.cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	directCmdList.graphic->OMSetRenderTargets(1, &currentBackBuffer->handle.cpuHandle, true, &swapChain->dsv->handle.cpuHandle);
}

void Graphics::Render(AppWindow* _window)
{
	// Per frame
	ID3D12DescriptorHeap* heap[] = { resourceManager.GetResourceHeap(), resourceManager.GetSamplerHeap() };
	directCmdList.graphic->SetDescriptorHeaps(2, heap);
	
	// Why does this result in the scene being rendered to the rtv of the imgui?
	//ui->BeginFrame();
	//ui->Update();
	//ui->Render(&directCmdList);

	

	directCmdList.graphic->SetPipelineState(pso.pipelineState);
	directCmdList.graphic->SetGraphicsRootSignature(bindlessSignature.signature);
	directCmdList.graphic->SetGraphicsRootDescriptorTable(0, resourceManager.GetTexture2DStartAddress());
	directCmdList.graphic->SetGraphicsRootConstantBufferView(3, camera->buffer->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootShaderResourceView(5, lManager->pLightList.GetLightsBufferPtr()->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootShaderResourceView(6, lManager->pLightList.GetLightsIndicesBufferPtr()->GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootConstantBufferView(7, lManager->numLightsCB.GetGPUAddress());
	directCmdList.graphic->SetGraphicsRootConstantBufferView(1, materialManager.GetMaterial<PhongMaterial>(testIDMaterial)->GetGPUAddress());

	directCmdList.graphic->SetGraphicsRootDescriptorTable(4, sampler->handle.gpuHandle);
	Matrix x = Matrix::Identity;
	static float temp = 0.f;
	x = Matrix::CreateRotationX(temp);
	static float pos = 0.f;
	//x *= Matrix::CreateTranslation(0, 0, pos);
	//testWorld.Update((void*)&x, sizeof(Matrix), frameIndex);
	temp += 0.0001f;
	static bool dir = false;

	if (dir)
	{
		if (pos > 1)
		{
			dir = false;
		}
		pos += 0.0001f;
	}
	else
	{
		if (pos < -1)
		{
			dir = true;
		}
		pos -= 0.0001f;
	}




	//std::cout << pos << std::endl;

	directCmdList.graphic->SetGraphicsRootConstantBufferView(2, testWorld.GetGPUAddress());

	// draw
	//directCmdList.graphic->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	directCmdList.graphic->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	directCmdList.graphic->IASetVertexBuffers(0, 1, &vbCache->GetBuffer(testMeshID)->GetView());
	//directCmdList.graphic->IASetIndexBuffer(&vbCache->GetBuffer(testMeshID)->GetIndexBuffer()->GetView());
	//directCmdList.graphic->DrawIndexedInstanced(vbCache->GetBuffer(testMeshID)->GetIndexBuffer()->numIndices, 1, 0, 0, 0);
	directCmdList.graphic->DrawInstanced(vbCache->GetBuffer(testMeshID)->GetNumVertices(), 1, 0, 0);
	//

	directCmdList.graphic->SetGraphicsRootConstantBufferView(2, testWorld2.GetGPUAddress());
	directCmdList.graphic->IASetVertexBuffers(0, 1, &vbCache->GetBuffer("sphere")->GetView());
	directCmdList.graphic->DrawInstanced(vbCache->GetBuffer("sphere")->GetNumVertices(), 1, 0, 0);

	ui->Update();
	ui->Render(&directCmdList);
}

void Graphics::Present()
{
	currentBackBuffer->Transition(directCmdList.graphic, D3D12_RESOURCE_STATE_PRESENT);
	nextSyncSignal = directCommandQueue->Execute(&directCmdList);

	// might enqueue work on gpu...
	swapChain->swapChain->Present(0, 0);
	
	if (frameCount % 3 == 0)
	{
		directCommandQueue->Flush(nextSyncSignal);
		allocator->allocator->Reset();
	}

	directCmdList.graphic->Reset(allocator->allocator, nullptr);

	frameCount++;
}
