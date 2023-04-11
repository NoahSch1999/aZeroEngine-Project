#include "RenderSystem.h"

void RendererSystem::Init(ID3D12Device* _device, ResourceEngine* _resourceEngine, VertexBufferCache* _vbCache,
	std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
{
	// Signature Setup
	componentMask.set(false);
	componentMask.set(COMPONENTENUM::TRANSFORM, true);
	componentMask.set(COMPONENTENUM::MESH, true);
	componentMask.set(COMPONENTENUM::MATERIAL, true);

	// Dependency Injection Setup
	resourceEngine = _resourceEngine;
	vbCache = _vbCache;
	lManager = _lManager;
	mManager = _mManager;
	swapChain = _swapChain;

	// Shared Resources
	solidRaster = RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT);
	wireFrameRaster = RasterState(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE);

	anisotropicWrapSampler.Init(_device, resourceEngine->GetDescriptorManager().GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	anisotropicBorderSampler.Init(_device, resourceEngine->GetDescriptorManager().GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

	InitShadowPass(_device);
	InitGeometryPass(_device);
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		ShadowPassBegin();
		GeometryPass();
	}
}

void RendererSystem::InitShadowPass(ID3D12Device* _device)
{
	RootParameters shadowParams;
	shadowParams.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world 0
	shadowParams.AddRootConstants(1, 16, D3D12_SHADER_VISIBILITY_VERTEX); // Light 1
	shadowRootSig.Init(_device, &shadowParams, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_ShadowPass.cso";
#ifdef _DEBUG
	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_ShadowPass.cso";
#endif // DEBUG

	shadowPso.Init(_device, &shadowRootSig, layout, solidRaster, swapChain->GetNumBackBuffers(), swapChain->GetBackBufferFormat(),
		DXGI_FORMAT_D24_UNORM_S8_UINT,vsPath, L"", L"", L"", L"");

	int sizeW = 4096;
	int sizeH = 4096;
	resourceEngine->CreateResource(shadowMap, sizeW, sizeH, true);

	lightViewPort.Height = (FLOAT)sizeH;
	lightViewPort.Width = (FLOAT)sizeW;

	lightViewPort.TopLeftX = 0;
	lightViewPort.TopLeftY = 0;
	lightViewPort.MinDepth = 0.0f;
	lightViewPort.MaxDepth = 1.0f;

	lightScizzorRect.left = 0;
	lightScizzorRect.top = 0;
	lightScizzorRect.right = sizeW;
	lightScizzorRect.bottom = sizeH;

#ifdef _DEBUG
	shadowPso.GetPipelineState()->SetName(L"Shadow Pass PSO");
	shadowRootSig.GetSignature()->SetName(L"Shadow Pass Root Signature");
	shadowMap.GetGPUOnlyResource()->SetName(L"Shadow map");
#endif // DEBUG
}

void RendererSystem::ShadowPassBegin()
{
	shadowMap.Transition(resourceEngine->renderPassList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	resourceEngine->renderPassList.GetGraphicList()->OMSetRenderTargets(0, NULL, true, &shadowMap.GetHandle().GetCPUHandleRef());
	resourceEngine->renderPassList.GetGraphicList()->ClearDepthStencilView(shadowMap.GetHandle().GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
	resourceEngine->renderPassList.GetGraphicList()->RSSetScissorRects(1, &lightScizzorRect);
	resourceEngine->renderPassList.GetGraphicList()->RSSetViewports(1, &lightViewPort);

	resourceEngine->renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	resourceEngine->renderPassList.GetGraphicList()->SetPipelineState(shadowPso.GetPipelineState());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootSignature(shadowRootSig.GetSignature());

	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(1, 16, (void*)&lManager->dLightData.VPMatrix, 0);

	for (Entity ent : entityIDMap.GetObjects())
	{
		Mesh* mesh = componentManager.GetComponent<Mesh>(ent);
		if (mesh->castShadows)
		{
			Transform* tf = componentManager.GetComponent<Transform>(ent);
			resourceEngine->renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID())->GetView());
			resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(0, tf->GetBuffer().GetGPUAddress());
			resourceEngine->renderPassList.GetGraphicList()->DrawInstanced(vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
		}
	}

	shadowMap.Transition(resourceEngine->renderPassList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RendererSystem::InitGeometryPass(ID3D12Device* _device)
{
	resourceEngine->CreateResource(geoPassDSV, swapChain->GetBackBufferDimensions().x, swapChain->GetBackBufferDimensions().y, false);


	// PBR Setup
	RootParameters paramsPBR;
	paramsPBR.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world matrix 0
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);	// camera 1
	paramsPBR.AddRootConstants(2, 8, D3D12_SHADER_VISIBILITY_PIXEL); // Camera ps 2
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0); // numlights 3
	paramsPBR.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// point light structs 4
	paramsPBR.AddRootConstants(5, 8, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// directional light cb 5
	paramsPBR.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL); // PBR Mat 6
	paramsPBR.AddRootConstants(3, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// Shadow Map 7
	paramsPBR.AddRootConstants(2, 16, D3D12_SHADER_VISIBILITY_VERTEX); // 8 Light Matrix
	paramsPBR.AddRootConstants(4, 1, D3D12_SHADER_VISIBILITY_PIXEL); // 9 perdrawconstants

	pbrRootSig.Init(_device, &paramsPBR, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_PBR.cso";
	std::wstring psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/PS_PBR.cso";
#ifdef _DEBUG
	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_PBR.cso";
	psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_PBR.cso";
#endif // _DEBUG

	pbrPso.Init(_device, &pbrRootSig, layout, solidRaster, swapChain->GetNumBackBuffers(), swapChain->GetBackBufferFormat(), geoPassDSV.GetFormat(),
		vsPath, psPath,
		L"", L"", L"");

#ifdef _DEBUG

	pbrPso.GetPipelineState()->SetName(L"Geometry Pass PBR PSO");
	pbrRootSig.GetSignature()->SetName(L"Geometry Pass PBR Root Signature");

	geoPassDSV.GetGPUOnlyResource()->SetName(L"Swap Chain Depth Stencil");
#endif // DEBUG
}

void RendererSystem::GeometryPass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	geoPassDSV.Clear(resourceEngine->renderPassList);
	resourceEngine->renderPassList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, &geoPassDSV.GetHandle().GetCPUHandleRef());
	resourceEngine->renderPassList.GetGraphicList()->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	resourceEngine->renderPassList.GetGraphicList()->RSSetScissorRects(1, &swapChain->GetScissorRect());
	resourceEngine->renderPassList.GetGraphicList()->RSSetViewports(1, &swapChain->GetViewPort());

	resourceEngine->renderPassList.GetGraphicList()->SetPipelineState(pbrPso.GetPipelineState());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootSignature(pbrRootSig.GetSignature());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(1, cam->GetBuffer().GetGPUAddress());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(3, lManager->numLightsCB.GetGPUAddress());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootShaderResourceView(4, lManager->pLightList.dataBuffer.GetGPUAddress());
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(5, 8, (void*)&lManager->dLightData.direction, 0);

	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(2, 4, (void*)&cam->GetForward(), 0);
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(2, 4, (void*)&cam->GetPosition(), 4);
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstant(7, shadowMap.GetSrvHandle().GetHeapIndex(), 0);
	resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0);

	for (Entity ent : entityIDMap.GetObjects())
	{
		resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRoot32BitConstant(9, componentManager.GetComponent<Mesh>(ent)->receiveShadows, 0);
		resourceEngine->renderPassList.GetGraphicList()->IASetVertexBuffers(0, 1, &vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID())->GetView());
		resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(0, componentManager.GetComponent<Transform>(ent)->GetBuffer().GetGPUAddress());
		resourceEngine->renderPassList.GetGraphicList()->SetGraphicsRootConstantBufferView(6, mManager->GetMaterial<PBRMaterial>(componentManager.GetComponent<MaterialComponent>(ent)->materialID)->GetGPUAddress());
		resourceEngine->renderPassList.GetGraphicList()->DrawInstanced(vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
	}
}