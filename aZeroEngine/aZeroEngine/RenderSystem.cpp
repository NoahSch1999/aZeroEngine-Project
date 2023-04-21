#include "RenderSystem.h"

void RendererSystem::Init(ID3D12Device* _device, ResourceEngine* _resourceEngine, VertexBufferCache* _vbCache,
	std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
{
	transformAllocator = std::make_unique<LinearResourceAllocator<Matrix>>(_device, 20000, 3);
	pbrMaterialAllocator = std::make_unique<LinearResourceAllocator<PBRMaterialInfo>>(_device, 500, 3);

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

	//InitShadowPass(_device);
	InitGeometryPass(_device);
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		transformAllocator->BeginFrame(resourceEngine->GetFrameIndex());
		pbrMaterialAllocator->BeginFrame(resourceEngine->GetFrameIndex());
		//ShadowPassBegin();
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
	/*shadowMap.Transition(resourceEngine->renderPassList, D3D12_RESOURCE_STATE_DEPTH_WRITE);

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
			resourceEngine->renderPassList.GetGraphicList()->DrawInstanced(vbCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID())->GetNumVertices(), 1, 0, 0);
		}
	}

	shadowMap.Transition(resourceEngine->renderPassList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);*/
}

void RendererSystem::InitGeometryPass(ID3D12Device* _device)
{
	resourceEngine->CreateResource(geoPassDSV, swapChain->GetBackBufferDimensions().x, swapChain->GetBackBufferDimensions().y, false);

	pickingRTV = std::make_shared<RenderTarget>();
	resourceEngine->CreateResource(*pickingRTV, swapChain->GetBackBufferDimensions().x, swapChain->GetBackBufferDimensions().y,
		 4, DXGI_FORMAT_R32_SINT, true, true, Vector4(-1, -1, -1, -1));

	// PBR Setup
	RootParameters paramsPBR;
	paramsPBR.AddRootConstants(0, 1, D3D12_SHADER_VISIBILITY_VERTEX); // per draw constants 0
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);	// camera 1
	paramsPBR.AddRootConstants(2, 8, D3D12_SHADER_VISIBILITY_PIXEL); // Camera ps 2
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0); // numlights 3
	paramsPBR.AddRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// point light structs 4
	paramsPBR.AddRootConstants(5, 8, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// directional light cb 5
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL); // Mat structured buffer 6
	paramsPBR.AddRootConstants(3, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// Shadow Map 7
	paramsPBR.AddRootConstants(2, 16, D3D12_SHADER_VISIBILITY_VERTEX); // 8 Light Matrix
	paramsPBR.AddRootConstants(4, 3, D3D12_SHADER_VISIBILITY_PIXEL); // 9 perdrawconstants (inc mat index)
	paramsPBR.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX); // 10 transform buffer

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
		L"", L"", L"", true);

#ifdef _DEBUG

	pbrPso.GetPipelineState()->SetName(L"Geometry Pass PBR PSO");
	pbrRootSig.GetSignature()->SetName(L"Geometry Pass PBR Root Signature");

	geoPassDSV.GetGPUOnlyResource()->SetName(L"Swap Chain Depth Stencil");
	pickingRTV->GetGPUOnlyResource()->SetName(L"Picking RTV");
#endif // DEBUG
}

void RendererSystem::GeometryPass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	std::shared_ptr<GraphicsCommandContext> context = resourceEngine->commandManager->GetGraphicsContext();

	ID3D12DescriptorHeap* heap[] = { resourceEngine->GetResourceHeap(), resourceEngine->GetSamplerHeap() };
	context->SetDescriptorHeaps(2, heap);

	D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetGPUOnlyResource().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	context->GetList().GetGraphicList()->ResourceBarrier(1, &r);

	context->ClearRenderTargetView(*currentBackBuffer);
	context->ClearRenderTargetView(*pickingRTV);
	context->ClearDepthStencilView(geoPassDSV);

	D3D12_CPU_DESCRIPTOR_HANDLE handle[2] = { currentBackBuffer->GetHandle().GetCPUHandle(), pickingRTV->GetHandle().GetCPUHandle() };

	context->SetOMRenderTargets(2, handle, false, &geoPassDSV.GetHandle().GetCPUHandleRef());
	context->SetIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->SetRSScizzorRects(1, &swapChain->GetScissorRect());
	context->SetRSViewports(1, &swapChain->GetViewPort());
	context->SetPipelineState(pbrPso.GetPipelineState());
	context->SetRootSignature(pbrRootSig.GetSignature());
	context->SetConstantBufferView(1, cam->GetBuffer().GetGPUAddress());
	context->SetConstantBufferView(3, lManager->numLightsCB.GetGPUAddress());
	context->SetShaderResourceView(4, lManager->pLightList.dataBuffer.GetGPUAddress());
	context->Set32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
	context->Set32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
	context->Set32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
	context->Set32BitRootConstant(7, shadowMap.GetSrvHandle().GetHeapIndex(), 0); // bug with 0 as offset?
	context->Set32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
	context->SetShaderResourceView(10, transformAllocator->GetVirtualAddress());
	context->SetShaderResourceView(6, pbrMaterialAllocator->GetVirtualAddress());

	PBRMaterial* defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	std::vector<Entity>& entities = entityIDMap.GetObjects();
	int numEntities = entities.size();

	Allocation<Matrix> transformAlloc = transformAllocator->Allocate(numEntities);

	Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->Allocate(mManager->GetPBRMaterials().size());
	int defaultMatIndex = pbrMatAlloc.CurrentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	for (int i = 0;i < numEntities; i++)
	{
		PixelDrawConstantsPBR perDraw;

		// Transform
		Transform* tf = componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		UINT tfID = transformAlloc.CurrentOffset();
		context->Set32BitRootConstant(0, tfID, 0);

		// Material
		// Reuse allocated material index
		PBRMaterial* mat = mManager->GetMaterial<PBRMaterial>(componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);
		if (!mat)
			perDraw.materialIndex = defaultMatIndex;
		else
		{
			perDraw.materialIndex = pbrMatAlloc.CurrentOffset();
			*pbrMatAlloc = mat->GetInfo();
			++pbrMatAlloc;
		}

		// Mesh
		perDraw.receiveShadows = componentManager.GetComponent<Mesh>(entities[i])->receiveShadows;

		// Other
		perDraw.pickingID = entities[i].id;

		context->Set32BitRootConstants(9, 3, (void*)&perDraw, 0);
		context->SetIAVertexBuffers(0, 1, &vbCache->GetResource(componentManager.GetComponent<Mesh>(entities[i])->GetID())->GetView());
		context->DrawInstanced(vbCache->GetResource(componentManager.GetComponent<Mesh>(entities[i])->GetID())->GetNumVertices(), 1, 0, 0);
		
		++transformAlloc;
	}

	pickingRTV->Transition(context->GetList(), D3D12_RESOURCE_STATE_COPY_SOURCE);

	context->CopyTextureToBuffer(resourceEngine->GetDevice(), pickingRTV->GetDesc(), pickingRTV->GetGPUOnlyResource().Get(),
		pickingRTV->GetReadbackResource().Get(), pickingRTV->GetGPUOnlyResourceState(), D3D12_RESOURCE_STATE_COMMON);

	pickingRTV->Transition(context->GetList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	resourceEngine->RequestReadback(pickingRTV);

	resourceEngine->commandManager->Execute(context);
}