#include "RenderSystem.h"

void RendererSystem::Init(ID3D12Device* _device, CommandManager& commandManager, ResourceTrashcan& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
	std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
{

	transformAllocator = std::make_unique<aZeroAlloc::LinearDefaultAllocator<DXM::Matrix>>(_device, 20000, 3);
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

	this->initGeometryPass(_device);
	this->initZPrePass(_device);
	this->initFXProcessGlow();

	RootParameters params;
	params.addRootConstants(0, 10);
	params.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_SRV);
	m_csRootSig.init(_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT 
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring csName = L"../x64/Release/CS_FXOutline.cso";
#ifdef _DEBUG
	csName = L"../x64/Debug/CS_FXOutline.cso";
#endif // DEBUG

	m_csPipeState = new ComputePipelineState(_device, m_csRootSig.getSignature(), csName);

	TextureSettings renderTextureSettings;
	renderTextureSettings.m_bytesPerTexel = 4;
	renderTextureSettings.m_clearValue.Color[0] = 0.3;
	renderTextureSettings.m_clearValue.Color[1] = 0.3;
	renderTextureSettings.m_clearValue.Color[2] = 0.3;
	renderTextureSettings.m_clearValue.Color[3] = 1;
	renderTextureSettings.m_clearValue.Format = _swapChain->getBackBufferFormat();
	renderTextureSettings.m_rtvFormat = _swapChain->getBackBufferFormat();
	renderTextureSettings.m_srvFormat = _swapChain->getBackBufferFormat();
	renderTextureSettings.m_width = _swapChain->getBackBufferDimensions().x;
	renderTextureSettings.m_height = _swapChain->getBackBufferDimensions().y;
	renderTextureSettings.m_flags = (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	renderTextureSettings.m_initialState = D3D12_RESOURCE_STATE_COMMON;

	m_renderTexture = std::move(Texture(_device, nullptr, renderTextureSettings, descriptorManager, trashcan));
	m_renderTexture.initUAV(_device, _swapChain->getBackBufferFormat());
	m_renderTexture.getGPUOnlyResource()->SetName(L"m_renderTexture");
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		transformAllocator->beginFrame(m_frameIndex);
		pbrMaterialAllocator->beginFrame(m_frameIndex);
		m_computeSelectionList->beginFrame(m_frameIndex);

		m_commandManager->graphicsWaitFor(m_commandManager->getCopyQueue(), m_commandManager->getCopyQueue().getLastSignalValue());
		if (m_prePassUsed)
		{
			this->passesIncPrepass();
		}
		else
		{
			this->passesNoPrepass();
		}

		if (m_blurByDistance)
		{
			this->fxProcessGlow();
		}
		
		if (m_drawSelectionOutlines)
		{
			if (!uiSelectionList->Empty())
			{
				this->postEffectOutlinePass();
			}
		}

		this->prepareBackbuffer();
	}
}

void RendererSystem::initShadowPass(ID3D12Device* _device)
{
//	RootParameters shadowParams;
//	shadowParams.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0); // world 0
//	shadowParams.addRootConstants(1, 16, D3D12_SHADER_VISIBILITY_VERTEX); // Light 1
//	shadowRootSig.init(_device, &shadowParams, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
//		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
//		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);
//
//	std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_ShadowPass.cso";
//#ifdef _DEBUG
//	vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Debug/VS_ShadowPass.cso";
//#endif // DEBUG
//
//	DXGI_FORMAT formats[] = { swapChain->getBackBufferFormat() };
//	shadowPso.init(_device, &shadowRootSig, layout.m_descs, 4, solidRaster, ARRAYSIZE(formats), formats,
//		DXGI_FORMAT_D24_UNORM_S8_UINT, vsPath, L"", L"", L"", L"");
//
//	const int sizeW = 4096;
//	const int sizeH = 4096;
//	GraphicsContextHandle context = m_commandManager->getGraphicsContext();
//
//	TextureSettings settings;
//	settings.m_clearValue.DepthStencil.Depth = 1;
//	settings.m_clearValue.DepthStencil.Stencil = 0;
//	settings.m_clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	settings.m_width = swapChain->getBackBufferDimensions().x;
//	settings.m_height = swapChain->getBackBufferDimensions().y;
//	settings.m_bytesPerTexel = 4;
//	settings.m_dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	settings.m_srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
//	settings.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
//	settings.m_flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
//	/*settings.m_createReadback = false;*/
//	shadowMap = std::move(Texture(_device, context.getList(), settings, *m_descriptorManager, *m_trashCan));
//
//	m_commandManager->executeContext(context);
//
//	lightViewPort.Height = (FLOAT)sizeH;
//	lightViewPort.Width = (FLOAT)sizeW;
//
//	lightViewPort.TopLeftX = 0;
//	lightViewPort.TopLeftY = 0;
//	lightViewPort.MinDepth = 0.0f;
//	lightViewPort.MaxDepth = 1.0f;
//
//	lightScizzorRect.left = 0;
//	lightScizzorRect.top = 0;
//	lightScizzorRect.right = sizeW;
//	lightScizzorRect.bottom = sizeH;
}

void RendererSystem::shadowPassBegin()
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

void RendererSystem::initGeometryPass(ID3D12Device* _device)
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

	geoPassDSV.getGPUOnlyResource()->SetName(L"geoPassDSV");

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

	pickingRTV->getGPUOnlyResource()->SetName(L"pickingRTV");
	m_readbackBuffer.getResource()->SetName(L"m_readbackBuffer");

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

	//std::wstring vsPath = L"C:/Projects/aZeroEngine/aZeroEngine/x64/Release/VS_PBR.cso";
	std::wstring vsPath = L"../x64/Release/VS_PBR.cso";
	std::wstring psPath = L"../x64/Release/PS_PBR.cso";
#ifdef _DEBUG
	vsPath = L"../x64/Debug/VS_PBR.cso";
	psPath = L"../x64/Debug/PS_PBR.cso";
#endif // _DEBUG

	CD3DX12_DEPTH_STENCIL_DESC dsvDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	dsvDesc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	InputLayout layout;
	layout.addElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	layout.addElement("UV", DXGI_FORMAT_R32G32_FLOAT);
	layout.addElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	layout.addElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);

	DXGI_FORMAT dsvFormat = geoPassDSV.getDSVFormat();
	DXGI_FORMAT formats[] = { swapChain->getBackBufferFormat(), DXGI_FORMAT_R32_SINT, swapChain->getBackBufferFormat() };

	PipelineState::PipelineStateDesc pbrPsoDesc;
	pbrPsoDesc.m_rootSignature = &pbrRootSig;
	pbrPsoDesc.m_inputLayout = &layout;
	pbrPsoDesc.m_rasterState = &solidRaster;

	pbrPsoDesc.m_rtvFormats = formats;
	pbrPsoDesc.m_numRTVFormats = 3;
	pbrPsoDesc.m_dsvFormat = geoPassDSV.getDSVFormat();
	pbrPsoDesc.m_vShaderPath = &vsPath;
	pbrPsoDesc.m_pShaderPath = &psPath;
	pbrPsoDesc.m_dsvDesc = &dsvDesc;

	pbrPso.init(_device, pbrPsoDesc);

	pbrPsoDesc.m_dsvDesc = nullptr;
	pbrPsoNoPrepass.init(_device, pbrPsoDesc);

	// Transparency PBR
	m_pbrRootSigTransp.init(_device, &paramsPBR, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	psPath = L"../x64/Release/PS_PBRTransparency.cso";
#ifdef _DEBUG
	psPath = L"../x64/Debug/PS_PBRTransparency.cso";
#endif // _DEBUG
	pbrPsoDesc.m_pShaderPath = &psPath;

	D3D12_RENDER_TARGET_BLEND_DESC rtvBlend;
	ZeroMemory(&rtvBlend, sizeof(D3D12_RENDER_TARGET_BLEND_DESC));
	rtvBlend.SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_COLOR;
	rtvBlend.DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR;
	rtvBlend.BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	rtvBlend.SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
	rtvBlend.DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_ZERO;
	rtvBlend.BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;

	rtvBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	rtvBlend.BlendEnable = true;
	rtvBlend.LogicOpEnable = false;
	
	D3D12_BLEND_DESC blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = true;
	blendDesc.RenderTarget[0] = rtvBlend;

	pbrPsoDesc.m_blendDesc = &blendDesc;
	m_pbrPsoTransp.init(_device, pbrPsoDesc);
}

void RendererSystem::passesNoPrepass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	GraphicsContextHandle opagueRenderContext = m_commandManager->getGraphicsContext();
	GraphicsContextHandle transRenderContext = m_commandManager->getGraphicsContext();
	GraphicsContextHandle* contexts[2] = { &opagueRenderContext, &transRenderContext };

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	contexts[0]->setDescriptorHeaps(2, heap);
	contexts[1]->setDescriptorHeaps(2, heap);

	contexts[0]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	contexts[0]->clearRenderTargetView(m_renderTexture);

	contexts[0]->transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	contexts[0]->clearRenderTargetView(m_glowRTVTexture);

	contexts[0]->clearRenderTargetView(*pickingRTV);
	contexts[0]->clearDepthStencilView(geoPassDSV);

	const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = { m_renderTexture.getRTVDSVHandle().getCPUHandle(),
		pickingRTV->getRTVDSVHandle().getCPUHandle(), m_glowRTVTexture.getRTVDSVHandle().getCPUHandle() };

	contexts[0]->setOMRenderTargets(3, rtvHandles, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());
	contexts[1]->setOMRenderTargets(3, rtvHandles, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());

	contexts[0]->setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	contexts[1]->setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	contexts[0]->setRSScizzorRects(1, &swapChain->getScissorRect());
	contexts[1]->setRSScizzorRects(1, &swapChain->getScissorRect());

	contexts[0]->setRSViewports(1, &swapChain->getViewPort());
	contexts[1]->setRSViewports(1, &swapChain->getViewPort());

	contexts[0]->setPipelineState(pbrPsoNoPrepass.getPipelineState());
	contexts[1]->setPipelineState(m_pbrPsoTransp.getPipelineState());

	contexts[0]->setGraphicsRootSignature(pbrRootSig.getSignature());
	contexts[1]->setGraphicsRootSignature(m_pbrRootSigTransp.getSignature());

	contexts[0]->setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
	contexts[1]->setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());

	contexts[0]->setConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());
	contexts[1]->setConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());

	contexts[0]->setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());
	contexts[1]->setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());

	contexts[0]->setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
	contexts[1]->setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);

	contexts[0]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
	contexts[1]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);

	contexts[0]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
	contexts[1]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);

	contexts[0]->setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?
	contexts[1]->setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?

	contexts[0]->setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
	contexts[1]->setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?

	contexts[0]->setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());
	contexts[1]->setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());

	contexts[0]->setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());
	contexts[1]->setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());

	PBRMaterial* const defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	std::vector<aZeroECS::Entity>& entities = m_entities.GetObjects();
	int numEntities = entities.size();

	aZeroAlloc::Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->getAllocation(numEntities);
	int defaultMatIndex = pbrMatAlloc.currentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	//
	aZeroAlloc::Allocation<DXM::Matrix> transformAlloc = transformAllocator->getAllocation(numEntities);
	transformAllocator->updateAll(contexts[0]->getList(), m_frameIndex);
	//
	int num[2]{ 0,0 };
	for (int i = 0; i < numEntities; i++)
	{
		PixelDrawConstantsPBR perDraw;

		PBRMaterial* const mat = mManager->GetMaterial<PBRMaterial>(m_componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);

		num[mat->GetInfo().enableTransparency]++;

		// Transform
		Transform* const tf = m_componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		tf->m_frameResourceIndex = transformAlloc.currentOffset();
		contexts[mat->GetInfo().enableTransparency]->setGraphics32BitRootConstant(0, tf->m_frameResourceIndex, 0);

		// Material
		// Reuse allocated material index
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

		contexts[mat->GetInfo().enableTransparency]->setGraphics32BitRootConstants(9, 3, (void*)&perDraw, 0);

		ModelAsset* model = modelCache->GetResource(m_componentManager.GetComponent<Mesh>(entities[i])->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->getVertexBufferView();
		contexts[mat->GetInfo().enableTransparency]->setIAVertexBuffers(0, 1, &vbView);
		D3D12_INDEX_BUFFER_VIEW ibView = model->getIndexBufferView();
		contexts[mat->GetInfo().enableTransparency]->setIAIndexBuffer(&ibView);

		contexts[mat->GetInfo().enableTransparency]->drawIndexedInstanced(model->getNumIndices(), 1, 0, 0, 0);

		++transformAlloc;
	}

	contexts[1]->transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	contexts[1]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//contexts[1]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	m_commandManager->executeContext(*contexts[0]);
	m_commandManager->executeContext(*contexts[1]);
}

void RendererSystem::passesIncPrepass()
{
	std::shared_ptr<Camera> cam = mainCamera.lock();

	GraphicsContextHandle prePassRenderContext = m_commandManager->getGraphicsContext();
	GraphicsContextHandle opagueRenderContext = m_commandManager->getGraphicsContext();
	GraphicsContextHandle transRenderContext = m_commandManager->getGraphicsContext();

	GraphicsContextHandle* contexts[3] = { &opagueRenderContext, &transRenderContext, &prePassRenderContext };
	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };

	const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = { m_renderTexture.getRTVDSVHandle().getCPUHandle(), 
		pickingRTV->getRTVDSVHandle().getCPUHandle(), m_glowRTVTexture.getRTVDSVHandle().getCPUHandle() };

	std::vector<aZeroECS::Entity>& entities = m_entities.GetObjects();
	int numEntities = entities.size();

	// Prep Transform Copy
	aZeroAlloc::Allocation<DXM::Matrix> transformAlloc = transformAllocator->getAllocation(numEntities);
	transformAllocator->updateAll(contexts[2]->getList(), m_frameIndex);

	// Z Pre-Pass
	{
		contexts[2]->setDescriptorHeaps(2, heap);
		contexts[2]->setPipelineState(zPassPSO.getPipelineState());
		contexts[2]->setGraphicsRootSignature(zPassRootSig.getSignature());
		contexts[2]->clearDepthStencilView(geoPassDSV);
		contexts[2]->setOMRenderTargets(0, nullptr, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());
		contexts[2]->setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contexts[2]->setRSScizzorRects(1, &swapChain->getScissorRect());
		contexts[2]->setRSViewports(1, &swapChain->getViewPort());
		contexts[2]->setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
		contexts[2]->setGraphicsShaderResourceView(2, transformAllocator->getVirtualAddress());
	}

	// Opaque Pass
	{
		contexts[0]->setDescriptorHeaps(2, heap);

		contexts[0]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		contexts[0]->clearRenderTargetView(m_renderTexture);

		contexts[0]->transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		contexts[0]->clearRenderTargetView(m_glowRTVTexture);

		contexts[0]->clearRenderTargetView(*pickingRTV);

		// Prep for pipeline
		contexts[0]->setOMRenderTargets(3, rtvHandles, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());
		contexts[0]->setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contexts[0]->setRSScizzorRects(1, &swapChain->getScissorRect());
		contexts[0]->setRSViewports(1, &swapChain->getViewPort());
		contexts[0]->setPipelineState(pbrPso.getPipelineState());
		contexts[0]->setGraphicsRootSignature(pbrRootSig.getSignature());
		contexts[0]->setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
		contexts[0]->setConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());
		contexts[0]->setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());
		contexts[0]->setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
		contexts[0]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
		contexts[0]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
		contexts[0]->setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?
		contexts[0]->setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
		contexts[0]->setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());
		contexts[0]->setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());
	}

	// Transparency Pass
	{
		contexts[1]->setDescriptorHeaps(2, heap);
		contexts[1]->setOMRenderTargets(3, rtvHandles, false, &geoPassDSV.getRTVDSVHandle().getCPUHandleRef());
		contexts[1]->setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		contexts[1]->setRSScizzorRects(1, &swapChain->getScissorRect());
		contexts[1]->setRSViewports(1, &swapChain->getViewPort());
		contexts[1]->setPipelineState(m_pbrPsoTransp.getPipelineState());
		contexts[1]->setGraphicsRootSignature(m_pbrRootSigTransp.getSignature());
		contexts[1]->setConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
		contexts[1]->setConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());
		contexts[1]->setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());
		contexts[1]->setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);
		contexts[1]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
		contexts[1]->setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
		contexts[1]->setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?
		contexts[1]->setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?
		contexts[1]->setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());
		contexts[1]->setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());
	}
	
	PBRMaterial* const defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	aZeroAlloc::Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->getAllocation(numEntities);
	int defaultMatIndex = pbrMatAlloc.currentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	for (int i = 0; i < numEntities; i++)
	{
		PBRMaterial* const mat = mManager->GetMaterial<PBRMaterial>(m_componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);

		// Transform Prep
		Transform* const tf = m_componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		tf->m_frameResourceIndex = transformAlloc.currentOffset();
		contexts[mat->GetInfo().enableTransparency]->setGraphics32BitRootConstant(0, tf->m_frameResourceIndex, 0);

		contexts[2]->setGraphics32BitRootConstant(0, tf->m_frameResourceIndex, 0);
		//

		// Per Draw Prep
		PixelDrawConstantsPBR perDraw;
		if (!mat)
		{
			perDraw.materialIndex = defaultMatIndex;
		}
		else
		{
			perDraw.materialIndex = pbrMatAlloc.currentOffset();
			*pbrMatAlloc = mat->GetInfo();
			++pbrMatAlloc;
		}
		perDraw.receiveShadows = m_componentManager.GetComponent<Mesh>(entities[i])->receiveShadows;
		perDraw.pickingID = entities[i].m_id;

		contexts[mat->GetInfo().enableTransparency]->setGraphics32BitRootConstants(9, 3, (void*)&perDraw, 0);
		//

		ModelAsset* model = modelCache->GetResource(m_componentManager.GetComponent<Mesh>(entities[i])->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->getVertexBufferView();

		D3D12_INDEX_BUFFER_VIEW ibView = model->getIndexBufferView();

		// Z Pre Pass
		{
			contexts[2]->setIAVertexBuffers(0, 1, &vbView);
			contexts[2]->setIAIndexBuffer(&ibView);
			contexts[2]->drawIndexedInstanced(model->getNumIndices(), 1, 0, 0, 0);
		}

		// Opague / Transparent Pass
		{
			contexts[mat->GetInfo().enableTransparency]->setIAVertexBuffers(0, 1, &vbView);
			contexts[mat->GetInfo().enableTransparency]->setIAIndexBuffer(&ibView);
			contexts[mat->GetInfo().enableTransparency]->drawIndexedInstanced(model->getNumIndices(), 1, 0, 0, 0);
		}

		++transformAlloc;
	}

	// Prepare for post FX pass
	//contexts[1]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	contexts[1]->transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	contexts[1]->transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// Execute Passes
	m_commandManager->executeContext(*contexts[2]);
	m_commandManager->executeContext(*contexts[0]);
	m_commandManager->executeContext(*contexts[1]);
}

void RendererSystem::initZPrePass(ID3D12Device* _device)
{
	// Create root sig
	RootParameters paramsZPreePass;
	paramsZPreePass.addRootConstants(0, 1, D3D12_SHADER_VISIBILITY_VERTEX); // 0 - Transform index
	paramsZPreePass.addRootDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX); // 1 - Camera cbv
	paramsZPreePass.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX); // 2 - Transform structured buffer

	zPassRootSig.init(_device, &paramsZPreePass, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	// Create pipeline state
	std::wstring vsPath = L"../x64/Release/VS_ZPrePass.cso";
#ifdef _DEBUG
	vsPath = L"../x64/Debug/VS_ZPrePass.cso";
#endif // _DEBUG

	InputLayout layout;
	layout.addElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);

	DXGI_FORMAT formats[] = { swapChain->getBackBufferFormat(), DXGI_FORMAT_R32_SINT };
	PipelineState::PipelineStateDesc zPrePassDesc;
	zPrePassDesc.m_rootSignature = &zPassRootSig;
	zPrePassDesc.m_inputLayout = &layout;
	zPrePassDesc.m_rasterState = &solidRaster;
	zPrePassDesc.m_dsvFormat = geoPassDSV.getDSVFormat();
	zPrePassDesc.m_vShaderPath = &vsPath;

	zPassPSO.init(_device, zPrePassDesc);
}

void RendererSystem::initFXProcessGlow()
{
	RootParameters params;
	params.addRootConstants(0, 5);
	m_processGlowRootSig.init(m_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	std::wstring csName = L"../x64/Release/CS_ProcessGlow.cso";
#ifdef _DEBUG
	csName = L"../x64/Debug/CS_ProcessGlow.cso";
#endif // DEBUG

	m_processGlowPipeState = new ComputePipelineState(m_device, m_processGlowRootSig.getSignature(), csName);

	TextureSettings settings;
	settings.m_srvFormat = swapChain->getBackBufferFormat();
	settings.m_width = swapChain->getBackBufferDimensions().x;
	settings.m_height = swapChain->getBackBufferDimensions().y;
	settings.m_bytesPerTexel = 4;
	settings.m_initialState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	settings.m_flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	m_glowTempTexture = std::move(Texture(m_device, nullptr, settings, *m_descriptorManager, *m_trashCan));
	m_glowTempTexture.initUAV(m_device, swapChain->getBackBufferFormat());
	
	settings.m_clearValue.Color[0] = 0.0f;
	settings.m_clearValue.Color[1] = 0.0f;
	settings.m_clearValue.Color[2] = 0.0f;
	settings.m_clearValue.Color[3] = 1.f;
	settings.m_clearValue.Format = swapChain->getBackBufferFormat();
	settings.m_rtvFormat = swapChain->getBackBufferFormat();
	settings.m_flags = (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	m_glowRTVTexture = std::move(Texture(m_device, nullptr, settings, *m_descriptorManager, *m_trashCan));
	m_glowRTVTexture.initUAV(m_device, swapChain->getBackBufferFormat());
	
	m_glowTempTexture.getGPUOnlyResource()->SetName(L"m_glowTempTexture");
	m_glowRTVTexture.getGPUOnlyResource()->SetName(L"m_glowRTVTexture");
}

void RendererSystem::fxProcessGlow()
{
	m_commandManager->copyWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());
	m_commandManager->computeWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());

	ComputeContextHandle context = m_commandManager->getComputeContext();
	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	context.setDescriptorHeaps(2, heap);
	context.setComputeRootSignature(m_processGlowRootSig.getSignature());
	context.setPipelineState(m_processGlowPipeState->getPipelineState());

	context.transitionTexture(m_glowTempTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	struct FxBlurData
	{
		int srcTextureID;
		int dstTextureID;
		int horizontal;
		int kernelWidth;
		int kernelHeight;
	};
	FxBlurData data;

	// src is glow texture
	// dst is empty glow result texture
	data.srcTextureID = m_glowRTVTexture.getSRVHandle().getHeapIndex();
	data.dstTextureID = m_glowTempTexture.getUAVHandle().getHeapIndex();

	if (m_blurDistRadiusX % 2 == 0)
	{
		m_blurDistRadiusX += 1;
	}

	if (m_blurDistRadiusY % 2 == 0)
	{
		m_blurDistRadiusY += 1;
	}
	
	data.kernelWidth = m_blurDistRadiusX;
	data.kernelHeight = m_blurDistRadiusY;

	data.horizontal = true;
	context.setCompute32BitRootConstants(0, 5, &data, 0);

	context.dispatch(std::ceil(m_renderTexture.getDimensions().x / 16.f), std::ceil(m_renderTexture.getDimensions().y / 16.f), 1);
	
	context.transitionTexture(m_glowTempTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	//context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// src is blur pass 1/2 horizontal texture
	// dst is frame color rtv
	data.srcTextureID = m_glowTempTexture.getSRVHandle().getHeapIndex();
	data.dstTextureID = m_renderTexture.getUAVHandle().getHeapIndex();

	data.horizontal = false;
	context.setCompute32BitRootConstants(0, 5, &data, 0);

	// needs m_renderTexture in uav mode
	// needs m_blurTempTexture in srv/generic-read mode
	context.dispatch(std::ceil(m_renderTexture.getDimensions().x / 16.f), std::ceil(m_renderTexture.getDimensions().y / 16.f), 1);

	m_commandManager->executeContext(context);
}

void RendererSystem::postEffectOutlinePass()
{
	m_commandManager->copyWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());
	m_commandManager->computeWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());

	{
		CopyContextHandle cContext = m_commandManager->getCopyContext();
		cContext.copyBufferRegion(transformAllocator->m_defaultBuffer.Get(), 0, transformAllocator->m_uploadBuffer.Get(), 0, 20000);
		cContext.copyBufferRegion(transformAllocator->m_defaultBuffer.Get(), 0, transformAllocator->m_uploadBuffer.Get(), 0, 20000);
		cContext.copyBufferRegion(transformAllocator->m_defaultBuffer.Get(), 0, transformAllocator->m_uploadBuffer.Get(), 0, 20000);
		m_commandManager->executeContext(cContext);
	}

	ComputeContextHandle context = m_commandManager->getComputeContext();
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
	data.mainOutlineColor = m_mainOutlineColor;
	data.secondOutlineColor = m_secondaryOutlineColor;
	data.outlineThickness = m_outlineThickness;
	data.renderTargetID = m_renderTexture.getUAVHandle().getHeapIndex();
	data.pickingTextureID = pickingRTV->getSRVHandle().getHeapIndex();

	context.setCompute32BitRootConstants(0, 10, &data, 0);
	context.setComputeShaderResourceView(1, m_computeSelectionList->getVirtualAddress());
	context.dispatch(std::ceil(m_renderTexture.getDimensions().x / 16.f), std::ceil(m_renderTexture.getDimensions().y / 16.f), 1);

	m_commandManager->executeContext(context);
}

void RendererSystem::prepareBackbuffer()
{
	m_commandManager->graphicsWaitFor(m_commandManager->getComputeQueue(), m_commandManager->getComputeQueue().getLastSignalValue());

	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
	context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	context.copyResource(currentBackBuffer->getGPUOnlyResource(), m_renderTexture.getGPUOnlyResource());

	m_commandManager->executeContext(context);
}
