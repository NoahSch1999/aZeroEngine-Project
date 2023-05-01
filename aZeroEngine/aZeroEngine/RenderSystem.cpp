#include "RenderSystem.h"

void RendererSystem::Init(ID3D12Device* _device, CommandManager& commandManager, ResourceTrashcan& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
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
	modelCache = _modelCache;
	lManager = _lManager;
	mManager = _mManager;
	swapChain = _swapChain;
	m_descriptorManager = &descriptorManager;
	m_device = _device;
	m_trashCan = &trashcan;
	m_commandManager = &commandManager;

	// Shared Resources
	solidRaster = RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT);
	wireFrameRaster = RasterState(D3D12_FILL_MODE_WIREFRAME, D3D12_CULL_MODE_NONE);

	anisotropicWrapSampler.Init(_device, descriptorManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	anisotropicBorderSampler.Init(_device, descriptorManager.GetSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

	//InitShadowPass(_device);
	InitGeometryPass(_device);
	InitOutlinePass(_device);
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		transformAllocator->BeginFrame(m_frameIndex);
		pbrMaterialAllocator->BeginFrame(m_frameIndex);
		//ShadowPassBegin();
		GeometryPass();
		if (!uiSelectionList->Empty())
		{
			OutlinePass();
		}
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

	DXGI_FORMAT formats[] = { swapChain->GetBackBufferFormat() };
	shadowPso.Init(_device, &shadowRootSig, layout, solidRaster, ARRAYSIZE(formats), formats,
		DXGI_FORMAT_D24_UNORM_S8_UINT, vsPath, L"", L"", L"", L"");

	const int sizeW = 4096;
	const int sizeH = 4096;
	GraphicsContextHandle context = m_commandManager->GetGraphicsContext();

	TextureSettings settings;
	settings.clearValue.DepthStencil.Depth = 1;
	settings.clearValue.DepthStencil.Stencil = 0;
	settings.clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.width = swapChain->GetBackBufferDimensions().x;
	settings.height = swapChain->GetBackBufferDimensions().y;
	settings.bytesPerTexel = 4;
	settings.dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	settings.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	settings.flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	settings.createReadback = false;
	shadowMap = std::move(Texture(_device, context.GetList(), settings, *m_descriptorManager, *m_trashCan));

	m_commandManager->ExecuteContext(context);

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
	GraphicsContextHandle context = m_commandManager->GetGraphicsContext();

	TextureSettings settings;
	settings.clearValue.DepthStencil.Depth = 1;
	settings.clearValue.DepthStencil.Stencil = 0;
	settings.clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.width = swapChain->GetBackBufferDimensions().x;
	settings.height = swapChain->GetBackBufferDimensions().y;
	settings.bytesPerTexel = 4;
	settings.dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	settings.flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	settings.createReadback = false;
	geoPassDSV = std::move(Texture(_device, context.GetList(), settings, *m_descriptorManager, *m_trashCan));

	TextureSettings settingsTwo;
	settingsTwo.clearValue.Color[0] = -1;
	settingsTwo.clearValue.Color[1] = -1;
	settingsTwo.clearValue.Color[2] = -1;
	settingsTwo.clearValue.Color[3] = -1;
	settingsTwo.clearValue.Format = DXGI_FORMAT_R32_SINT;
	settingsTwo.width = swapChain->GetBackBufferDimensions().x;
	settingsTwo.height = swapChain->GetBackBufferDimensions().y;
	settingsTwo.bytesPerTexel = 4;
	settingsTwo.rtvFormat = DXGI_FORMAT_R32_SINT;
	settingsTwo.initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	settingsTwo.flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	settingsTwo.createReadback = true;
	pickingRTV = std::make_shared<Texture>();
	*pickingRTV = std::move(Texture(_device, context.GetList(), settingsTwo, *m_descriptorManager, *m_trashCan));

	m_commandManager->ExecuteContext(context);

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

	DXGI_FORMAT formats[] = { swapChain->GetBackBufferFormat(), DXGI_FORMAT_R32_SINT };
	pbrPso.Init(_device, &pbrRootSig, layout, solidRaster, ARRAYSIZE(formats), formats, geoPassDSV.GetDSVFormat(),
		vsPath, psPath,
		L"", L"", L"", true);
}

void RendererSystem::GeometryPass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	GraphicsContextHandle context = m_commandManager->GetGraphicsContext();

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->GetResourceHeap(), m_descriptorManager->GetSamplerHeap()};
	context.SetDescriptorHeaps(2, heap);

	context.TransitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

	context.ClearRenderTargetView(*currentBackBuffer);
	context.ClearRenderTargetView(*pickingRTV);
	context.ClearDepthStencilView(geoPassDSV);

	const D3D12_CPU_DESCRIPTOR_HANDLE handle[2] = { currentBackBuffer->GetRTVDSVHandle().GetCPUHandle(), pickingRTV->GetRTVDSVHandle().GetCPUHandle() };

	context.SetOMRenderTargets(2, handle, false, &geoPassDSV.GetRTVDSVHandle().GetCPUHandleRef());
	context.SetIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context.SetRSScizzorRects(1, &swapChain->GetScissorRect());
	context.SetRSViewports(1, &swapChain->GetViewPort());
	context.SetPipelineState(pbrPso.GetPipelineState());
	context.SetRootSignature(pbrRootSig.GetSignature());
	context.SetConstantBufferView(1, cam->GetBuffer()->GetVirtualAddress());
	context.SetConstantBufferView(3, lManager->numLightsCB->GetVirtualAddress());
	context.SetShaderResourceView(4, lManager->pLightList.dataBuffer.GetVirtualAddress());
	context.Set32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
	context.Set32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
	context.Set32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
	context.Set32BitRootConstant(7, shadowMap.GetSRVHandle().GetHeapIndex(), 0); // bug with 0 as offset?
	context.Set32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
	context.SetShaderResourceView(10, transformAllocator->GetVirtualAddress());
	context.SetShaderResourceView(6, pbrMaterialAllocator->GetVirtualAddress());

	PBRMaterial* const defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	std::vector<Entity>& entities = entityIDMap.GetObjects();
	int numEntities = entities.size();

	Allocation<Matrix> transformAlloc = transformAllocator->Allocate(numEntities);

	Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->Allocate(mManager->GetPBRMaterials().size());
	int defaultMatIndex = pbrMatAlloc.CurrentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	for (int i = 0; i < numEntities; i++)
	{
		PixelDrawConstantsPBR perDraw;

		// Transform
		Transform* const tf = componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		UINT tfID = transformAlloc.CurrentOffset();
		context.Set32BitRootConstant(0, tfID, 0);

		// Material
		// Reuse allocated material index
		PBRMaterial* const mat = mManager->GetMaterial<PBRMaterial>(componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);
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

		context.Set32BitRootConstants(9, 3, (void*)&perDraw, 0);

		ModelAsset* model = modelCache->GetResource(componentManager.GetComponent<Mesh>(entities[i])->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->GetVertexBufferView();
		context.SetIAVertexBuffers(0, 1, &vbView);
		D3D12_INDEX_BUFFER_VIEW ibView = model->GetIndexBufferView();
		context.SetIAIndexBuffer(&ibView);

		context.DrawIndexedInstanced(model->GetNumIndices(), 1, 0, 0, 0);

		++transformAlloc;
	}

	context.TransitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_COPY_SOURCE);
	context.CopyTextureToBuffer(m_device, pickingRTV->GetGPUOnlyResource(), pickingRTV->GetReadbackResource());
	context.TransitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_commandManager->ExecuteContext(context);
}

void RendererSystem::InitOutlinePass(ID3D12Device* _device)
{
	RootParameters outlineParams;
	outlineParams.AddRootConstants(0, 16, D3D12_SHADER_VISIBILITY_VERTEX); // matrice 0
	outlineParams.AddRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // cam 1
	outlineParams.AddRootConstants(0, 4, D3D12_SHADER_VISIBILITY_PIXEL); // colorConstant 2
	outlineRoot.Init(_device, &outlineParams, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_Outline.cso";
#ifdef _DEBUG
	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_Outline.cso";
#endif // DEBUG

	std::wstring psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/PS_Outline.cso";
#ifdef _DEBUG
	psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_Outline.cso";
#endif // DEBUG

	DXGI_FORMAT formats[] = { swapChain->GetBackBufferFormat() };
	outlinePSO.Init(_device, &outlineRoot, layout, solidRaster, ARRAYSIZE(formats), formats,
		DXGI_FORMAT_D24_UNORM_S8_UINT, vsPath, psPath, L"", L"", L"", false, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
}

void RendererSystem::OutlinePass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	GraphicsContextHandle context = m_commandManager->GetGraphicsContext();

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->GetResourceHeap(), m_descriptorManager->GetSamplerHeap() };
	context.SetDescriptorHeaps(2, heap);

	context.SetOMRenderTargets(1, &currentBackBuffer->GetRTVDSVHandle().GetCPUHandleRef(), false, &geoPassDSV.GetRTVDSVHandle().GetCPUHandleRef());
	context.SetIAPrimiteTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	context.SetRSScizzorRects(1, &swapChain->GetScissorRect());
	context.SetRSViewports(1, &swapChain->GetViewPort());
	context.SetPipelineState(outlinePSO.GetPipelineState());
	context.SetRootSignature(outlineRoot.GetSignature());

	std::list<int>::iterator beg = uiSelectionList->Begin();
	std::list<int>::iterator end = uiSelectionList->End();
	int renderCount = 0;
	for (auto i = beg; i!= end; i++)
	{
		Entity& ent = currentScene->GetEntity(*i);
		Transform* tf = componentManager.GetComponent<Transform>(ent);
		Mesh* mesh = componentManager.GetComponent<Mesh>(ent);

		if (!tf || !mesh)
		{
			renderCount++;
			continue;
		}

		if (renderCount == 0)
		{

			const Vector4 colorConstants = { 1.f, 0.f, 0.f, 1.f };
			context.Set32BitRootConstants(2, 4, &colorConstants, 0);
		}
		else
		{

			const Vector4 colorConstants = { 1.f, 1.f, 0.f, 1.f };
			context.Set32BitRootConstants(2, 4, &colorConstants, 0);
		}
		
		const Matrix world = Matrix::CreateScale(1.0001f) * tf->GetWorldMatrix();

		context.Set32BitRootConstants(0, 16, &world, 0);
		context.SetConstantBufferView(1, cam->GetBuffer()->GetVirtualAddress());

		ModelAsset* model = modelCache->GetResource(componentManager.GetComponent<Mesh>(ent)->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->GetVertexBufferView();
		context.SetIAVertexBuffers(0, 1, &vbView);
		D3D12_INDEX_BUFFER_VIEW ibView = model->GetIndexBufferView();
		context.SetIAIndexBuffer(&ibView);

		context.DrawIndexedInstanced(model->GetNumIndices(), 1, 0, 0, 0);

		renderCount++;
	}
	
	m_commandManager->ExecuteContext(context);
}
