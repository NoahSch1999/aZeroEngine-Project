#include "Graphics.h"
#include <array>

Graphics::Graphics(AppWindow* _window, HINSTANCE _instance)
	:frameCount(0), frameIndex(0), texture(), texturex()
{
	Initialize(_window, _instance);
}

Graphics::~Graphics()
{
	delete directCommandQueue;
	delete swapChain;
	delete rasterState;
	device->Release();
	delete cb, sampler;
	delete rtvHeap, dsvHeap, resourceHeap, stagingHeap;
	delete allocator/*, directCommandQueue*/;
	delete camera->buffer, world.buffer;
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

	// SwapChain
	rtvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, L"RTV Heap");
	dsvHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, L"DSV Heap");
	resourceHeap = new ShaderDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 6, L"SHADER Heap");
	samplerHeap = new ShaderDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 4, L"SAMPLER Heap");
	stagingHeap = new HiddenDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5, L"STAGING Heap");
	allocator = new CommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	allocator->InitCommandList(device, &directCmdList);

	swapChain = new SwapChain(device, directCommandQueue, &directCmdList, dsvHeap, rtvHeap, _window->windowHandle, _window->width, _window->height, 3, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);

	texture.Init(device, resourceHeap, &directCmdList, "C:/Projects/aZeroEngine/aZeroEngine/textures/sadcat.png");
	texturex.Init(device, resourceHeap, &directCmdList, "C:/Projects/aZeroEngine/aZeroEngine/textures/pylotTest.png");
	mesh.LoadBufferFromFile(device, &directCmdList, "C:/Projects/aZeroEngine/aZeroEngine/meshes/cube");

	rasterState = new RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);

	sampler = new Sampler(device, samplerHeap, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

	RootParameters params;
	params.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
	params.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);
	params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);
	params.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);

	signature.Initialize(device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, 0, nullptr);

	pso.Init(device, &signature, &layout, rasterState, swapChain->numBackBuffers, swapChain->rtvFormat, swapChain->dsvFormat,
		L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Basic.cso", L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Basic.cso",
		L"", L"", L"");

	int nextSignal = directCommandQueue->Execute(&directCmdList, 1);
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);

	//
	camera = new Camera(device, resourceHeap, &directCmdList, _window->width, _window->height, _instance, _window->windowHandle);
	
	world.pos = Vector3(0, 0, 0);

	world.world = Matrix::CreateScale(0.4f);
	world.world *= Matrix::CreateTranslation(0, 0, 1);
	world.world.Transpose();

	world.buffer = new ConstantBuffer(device, resourceHeap, &directCmdList, (void*)&world.world, sizeof(Matrix), false, L"World");
	//

	mesh.buffer->resource->SetName(L"Mesh");


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

void Graphics::Update(AppWindow* _window)
{
	// Per frame
	ID3D12DescriptorHeap* heap[] = { resourceHeap->heap, samplerHeap->heap };
	directCmdList.graphic->SetPipelineState(pso.pipelineState);
	directCmdList.graphic->SetDescriptorHeaps(2, heap);
	directCmdList.graphic->SetGraphicsRootSignature(signature.signature);
	directCmdList.graphic->SetGraphicsRootConstantBufferView(0, world.buffer->gpuAddress);
	directCmdList.graphic->SetGraphicsRootConstantBufferView(1, camera->buffer->gpuAddress);
	directCmdList.graphic->SetGraphicsRootDescriptorTable(2, texture.sResource->handle.gpuHandle);
	directCmdList.graphic->SetGraphicsRootDescriptorTable(3, sampler->handle.gpuHandle);

	directCmdList.graphic->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	directCmdList.graphic->IASetVertexBuffers(0, 1, &mesh.buffer->view);
	directCmdList.graphic->DrawInstanced(mesh.numVertices, 1, 0, 0);
}

void Graphics::Present()
{
	currentBackBuffer->Transition(directCmdList.graphic, D3D12_RESOURCE_STATE_PRESENT);
	int nextSignal = directCommandQueue->Execute(&directCmdList, 1);
	swapChain->swapChain->Present(0, 0);
	// DXGI ERROR: IDXGISwapChain::Present: The application has not called ResizeBuffers or re-created the SwapChain after a fullscreen or windowed transition. 
	//		Flip model swapchains (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL and DXGI_SWAP_EFFECT_FLIP_DISCARD) are required to do so. [ MISCELLANEOUS ERROR #117: ]
	directCommandQueue->Flush(nextSignal, allocator, directCmdList.graphic);
	frameCount++;
}

void Graphics::SyncProcessors()
{
	directCommandQueue->Flush();
}

void Graphics::SyncProcessors(CommandQueue* _cmdQueue)
{
	_cmdQueue->Flush();
}
