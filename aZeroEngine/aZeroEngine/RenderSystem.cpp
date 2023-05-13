#include "RenderSystem.h"

void RendererSystem::Init(ID3D12Device* _device, CommandManager& commandManager, ResourceTrashcan& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
	std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
{

	transformAllocator = std::make_unique<aZeroAlloc::LinearUploadAllocator<DXM::Matrix>>(_device, 20000, 3);
	pbrMaterialAllocator = std::make_unique<aZeroAlloc::LinearUploadAllocator<PBRMaterialInfo>>(_device, 500, 3);
	m_computeSelectionList = std::make_unique<aZeroAlloc::LinearUploadAllocator<int>>(_device, 200, 3);

	// Signature Setup
	m_componentMask.set(m_componentManager.GetComponentBit<Transform>());
	m_componentMask.set(m_componentManager.GetComponentBit<Mesh>());
	m_componentMask.set(m_componentManager.GetComponentBit<MaterialComponent>());
	/*componentMask.set(false);
	componentMask.set(COMPONENTENUM::TRANSFORM, true);
	componentMask.set(COMPONENTENUM::MESH, true);
	componentMask.set(COMPONENTENUM::MATERIAL, true);*/

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

	anisotropicWrapSampler.init(_device, descriptorManager.getSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	anisotropicBorderSampler.init(_device, descriptorManager.getSamplerDescriptor(), D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

	this->InitGeometryPass(_device);

	RootParameters params;
	params.addRootConstants(0, 10);
	params.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_SRV);
	m_csRootSig.init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT 
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring csName = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/CS_FXOutline.cso";
#ifdef _DEBUG
	csName = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/CS_FXOutline.cso";
#endif // DEBUG

	m_csPipeState = new ComputePipelineState(_device, m_csRootSig.getSignature(), csName);

	TextureSettings renderTextureSettings;
	renderTextureSettings.m_bytesPerTexel = 4;
	renderTextureSettings.m_clearValue.Color[0] = 0;
	renderTextureSettings.m_clearValue.Color[1] = 0;
	renderTextureSettings.m_clearValue.Color[2] = 0;
	renderTextureSettings.m_clearValue.Color[3] = 0;
	renderTextureSettings.m_clearValue.Format = _swapChain->getBackBufferFormat();
	renderTextureSettings.m_rtvFormat = _swapChain->getBackBufferFormat();
	renderTextureSettings.m_width = _swapChain->getBackBufferDimensions().x;
	renderTextureSettings.m_height = _swapChain->getBackBufferDimensions().y;
	renderTextureSettings.m_flags = (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	renderTextureSettings.m_initialState = D3D12_RESOURCE_STATE_COMMON;

	/*m_renderTexture.resize(3);*/
	m_renderTexture = std::move(Texture(_device, nullptr, renderTextureSettings, descriptorManager, trashcan));
	m_renderTexture.initUAV(_device, _swapChain->getBackBufferFormat());

	/*m_renderTexture[1] = std::move(Texture(_device, nullptr, renderTextureSettings, descriptorManager, trashcan));
	m_renderTexture[1].initUAV(_device, _swapChain->getBackBufferFormat());

	m_renderTexture[2] = std::move(Texture(_device, nullptr, renderTextureSettings, descriptorManager, trashcan));
	m_renderTexture[2].initUAV(_device, _swapChain->getBackBufferFormat());*/

	printf("x");
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		transformAllocator->beginFrame(m_frameIndex);
		pbrMaterialAllocator->beginFrame(m_frameIndex);
		m_computeSelectionList->beginFrame(m_frameIndex);

		GeometryPass();
		if (!uiSelectionList->Empty())
		{
			PostEffectOutlinePass();
		}

		PrepareBackbuffer();

		{
			GraphicsContextHandle context = m_commandManager->getGraphicsContext();
			context.transitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_COPY_SOURCE);
			context.copyTextureToBuffer(m_device, pickingRTV->getGPUOnlyResource(), m_readbackBuffer.getResource());
			context.transitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_commandManager->executeContext(context);
		}
	}
}

void RendererSystem::InitShadowPass(ID3D12Device* _device)
{
	RootParameters shadowParams;
	shadowParams.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world 0
	shadowParams.addRootConstants(1, 16, D3D12_SHADER_VISIBILITY_VERTEX); // Light 1
	shadowRootSig.init(_device, &shadowParams, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_ShadowPass.cso";
#ifdef _DEBUG
	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_ShadowPass.cso";
#endif // DEBUG

	DXGI_FORMAT formats[] = { swapChain->getBackBufferFormat() };
	shadowPso.init(_device, &shadowRootSig, layout, solidRaster, ARRAYSIZE(formats), formats,
		DXGI_FORMAT_D24_UNORM_S8_UINT, vsPath, L"", L"", L"", L"");

	const int sizeW = 4096;
	const int sizeH = 4096;
	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	TextureSettings settings;
	settings.m_clearValue.DepthStencil.Depth = 1;
	settings.m_clearValue.DepthStencil.Stencil = 0;
	settings.m_clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.m_width = swapChain->getBackBufferDimensions().x;
	settings.m_height = swapChain->getBackBufferDimensions().y;
	settings.m_bytesPerTexel = 4;
	settings.m_dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.m_srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	settings.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	settings.m_flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	/*settings.m_createReadback = false;*/
	shadowMap = std::move(Texture(_device, context.getList(), settings, *m_descriptorManager, *m_trashCan));

	m_commandManager->executeContext(context);

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
	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	TextureSettings settings;
	settings.m_clearValue.DepthStencil.Depth = 1;
	settings.m_clearValue.DepthStencil.Stencil = 0;
	settings.m_clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.m_width = swapChain->getBackBufferDimensions().x;
	settings.m_height = swapChain->getBackBufferDimensions().y;
	settings.m_bytesPerTexel = 4;
	settings.m_dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	settings.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	settings.m_flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	geoPassDSV = std::move(Texture(_device, context.getList(), settings, *m_descriptorManager, *m_trashCan));

	TextureSettings settingsTwo;
	settingsTwo.m_clearValue.Color[0] = -1;
	settingsTwo.m_clearValue.Color[1] = -1;
	settingsTwo.m_clearValue.Color[2] = -1;
	settingsTwo.m_clearValue.Color[3] = -1;
	settingsTwo.m_clearValue.Format = DXGI_FORMAT_R32_SINT;
	settingsTwo.m_width = swapChain->getBackBufferDimensions().x;
	settingsTwo.m_height = swapChain->getBackBufferDimensions().y;
	settingsTwo.m_bytesPerTexel = 4;
	settingsTwo.m_rtvFormat = DXGI_FORMAT_R32_SINT;
	settingsTwo.m_srvFormat = DXGI_FORMAT_R32_SINT;
	settingsTwo.m_initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	settingsTwo.m_flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	pickingRTV = std::make_shared<Texture>();
	*pickingRTV = std::move(Texture(_device, context.getList(), settingsTwo, *m_descriptorManager, *m_trashCan));
	m_readbackBuffer = std::move(ReadbackBuffer(_device, *m_trashCan, pickingRTV->getPaddedRowPitch(), pickingRTV->getDimensions().y));

	m_commandManager->executeContext(context);

	// PBR Setup
	RootParameters paramsPBR;
	paramsPBR.addRootConstants(0, 1, D3D12_SHADER_VISIBILITY_VERTEX); // per draw constants 0
	paramsPBR.addRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX);	// camera 1
	paramsPBR.addRootConstants(2, 8, D3D12_SHADER_VISIBILITY_PIXEL); // Camera ps 2
	paramsPBR.addRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0); // numlights 3
	paramsPBR.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// point light structs 4
	paramsPBR.addRootConstants(5, 8, D3D12_SHADER_VISIBILITY_PIXEL, 0);	// directional light cb 5
	paramsPBR.addRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_PIXEL); // Mat structured buffer 6
	paramsPBR.addRootConstants(3, 1, D3D12_SHADER_VISIBILITY_PIXEL);	// Shadow Map 7
	paramsPBR.addRootConstants(2, 16, D3D12_SHADER_VISIBILITY_VERTEX); // 8 Light Matrix
	paramsPBR.addRootConstants(4, 3, D3D12_SHADER_VISIBILITY_PIXEL); // 9 perdrawconstants (inc mat index)
	paramsPBR.addRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX); // 10 transform buffer

	pbrRootSig.init(_device, &paramsPBR, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_PBR.cso";
	std::wstring psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/PS_PBR.cso";
#ifdef _DEBUG
	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_PBR.cso";
	psPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/PS_PBR.cso";
#endif // _DEBUG

	DXGI_FORMAT formats[] = { swapChain->getBackBufferFormat(), DXGI_FORMAT_R32_SINT };
	pbrPso.init(_device, &pbrRootSig, layout, solidRaster, ARRAYSIZE(formats), formats, geoPassDSV.getDSVFormat(),
		vsPath, psPath,
		L"", L"", L"", true);
}

void RendererSystem::GeometryPass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap()};
	context.setDescriptorHeaps(2, heap);

	context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	context.clearRenderTargetView(*currentBackBuffer);

	context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);

	context.clearRenderTargetView(m_renderTexture);
	context.clearRenderTargetView(*pickingRTV);
	context.clearDepthStencilView(geoPassDSV);

	const D3D12_CPU_DESCRIPTOR_HANDLE handle[2] = { m_renderTexture.getRTVDSVHandle().getCPUHandle(), pickingRTV->getRTVDSVHandle().getCPUHandle()};

	context.setOMRenderTargets(2, handle, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());
	context.setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context.setRSScizzorRects(1, &swapChain->getScissorRect());
	context.setRSViewports(1, &swapChain->getViewPort());
	context.setPipelineState(pbrPso.getPipelineState());
	context.setGraphicsRootSignature(pbrRootSig.getSignature());
	context.setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
	context.setConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());
	context.setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());
	context.setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
	context.setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
	context.setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
	context.setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?
	context.setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
	context.setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());
	context.setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());

	PBRMaterial* const defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	std::vector<aZeroECS::Entity>& entities = m_entities.GetObjects();
	int numEntities = entities.size();

	aZeroAlloc::Allocation<DXM::Matrix> transformAlloc = transformAllocator->getAllocation(numEntities);

	aZeroAlloc::Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->getAllocation(numEntities);
	int defaultMatIndex = pbrMatAlloc.currentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	for (int i = 0; i < numEntities; i++)
	{
		PixelDrawConstantsPBR perDraw;

		// Transform
		Transform* const tf = m_componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		UINT tfID = transformAlloc.currentOffset();
		context.setGraphics32BitRootConstant(0, tfID, 0);

		// Material
		// Reuse allocated material index
		PBRMaterial* const mat = mManager->GetMaterial<PBRMaterial>(m_componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);
		if (!mat)
			perDraw.materialIndex = defaultMatIndex;
		else
		{
			perDraw.materialIndex = pbrMatAlloc.currentOffset();
			*pbrMatAlloc = mat->GetInfo();
			++pbrMatAlloc;
		}

		// Mesh
		perDraw.receiveShadows = m_componentManager.GetComponent<Mesh>(entities[i])->receiveShadows;

		// Other
		perDraw.pickingID = entities[i].m_id;

		context.setGraphics32BitRootConstants(9, 3, (void*)&perDraw, 0);

		ModelAsset* model = modelCache->GetResource(m_componentManager.GetComponent<Mesh>(entities[i])->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->getVertexBufferView();
		context.setIAVertexBuffers(0, 1, &vbView);
		D3D12_INDEX_BUFFER_VIEW ibView = model->getIndexBufferView();
		context.setIAIndexBuffer(&ibView);

		context.drawIndexedInstanced(model->getNumIndices(), 1, 0, 0, 0);

		++transformAlloc;
	}

	context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	m_commandManager->executeContext(context);
}

void RendererSystem::PostEffectOutlinePass()
{
	GraphicsContextHandle context = m_commandManager->getGraphicsContext();
	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	context.setDescriptorHeaps(2, heap);
	context.setComputeRootSignature(m_csRootSig.getSignature());
	context.setPipelineState(m_csPipeState->getPipelineState());
	
	struct OutlinePassData
	{
		DXM::Vector3 mainOutlineColor = { 1,0,0 };
		int outlineThickness = 2;
		DXM::Vector3 secondOutlineColor = { 1,1,0 };
		int renderTargetID = 0;
		int pickingTextureID = 0;
		int numSelections = -1;
	};
	
	std::list<int>::iterator beg = uiSelectionList->Begin();
	std::list<int>::iterator end = uiSelectionList->End();
	
	// TO DO: Handle the case where the number of selected entities are more than the number of allocated space for m_computeSelectionList 
	aZeroAlloc::Allocation<int> allocation = m_computeSelectionList->getAllocation(uiSelectionList->Count());
	for (auto i = beg; i != end; i++)
	{
		*allocation = *i;
		++allocation;
	}

	aZeroECS::Entity& ent = currentScene->GetEntity(*beg);
	OutlinePassData data;
	data.numSelections = uiSelectionList->Count();
	data.renderTargetID = m_renderTexture.getUAVHandle().getHeapIndex();
	data.pickingTextureID = pickingRTV->getSRVHandle().getHeapIndex();

	context.setCompute32BitRootConstants(0, 10, &data, 0);
	context.setComputeShaderResourceView(1, m_computeSelectionList->getVirtualAddress());
	context.dispatch(std::ceil(m_renderTexture.getDimensions().x / 16.f), std::ceil(m_renderTexture.getDimensions().y / 16.f), 1);

	m_commandManager->executeContext(context);
}

void RendererSystem::PrepareBackbuffer()
{
	GraphicsContextHandle context = m_commandManager->getGraphicsContext();
	
	context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
	context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	context.copyResource(currentBackBuffer->getGPUOnlyResource(), m_renderTexture.getGPUOnlyResource());
	//context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	/*currentBackBuffer->setResourceState(D3D12_RESOURCE_STATE_COPY_DEST);*/
	
	m_commandManager->executeContext(context);
}
