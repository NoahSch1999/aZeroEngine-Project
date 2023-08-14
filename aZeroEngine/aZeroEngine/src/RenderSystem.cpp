#include "RenderSystem.h"
#include "GraphicsMemory.h"

void RendererSystem::Init(ID3D12Device* _device, CommandManager& commandManager, ResourceRecycler& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
	std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, ShaderCache* shaderCache, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
{
	transformAllocator = std::make_unique<aZeroAlloc::LinearDefaultAllocator<DXM::Matrix>>(_device, 20000, 3);
	pbrMaterialAllocator = std::make_unique<aZeroAlloc::LinearUploadAllocator<PBRMaterialInfo>>(_device, 500, 3);
	m_computeSelectionList = std::make_unique<aZeroAlloc::LinearUploadAllocator<int>>(_device, 200, 3);

	// Signature Setup
	m_componentMask.set(m_componentManager.GetComponentBit<Transform>());
	m_componentMask.set(m_componentManager.GetComponentBit<Mesh>());
	m_componentMask.set(m_componentManager.GetComponentBit<MaterialComponent>());

	// Dependency Injection Setup
	modelCache = _modelCache;
	lManager = _lManager;
	mManager = _mManager;
	swapChain = _swapChain;
	m_descriptorManager = &descriptorManager;
	m_device = _device;
	m_trashCan = &trashcan;
	m_commandManager = &commandManager;
	m_shaderCache = shaderCache;

	// Shared Resources
	solidRaster = RasterState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT);

	m_samplers[RenderSettings::POINT] = std::move(Sampler(_device, descriptorManager, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP));

	m_samplers[RenderSettings::LINEAR] = std::move(Sampler(_device, descriptorManager, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP));

	m_samplers[RenderSettings::ANISOTROPIC_X8] = std::move(Sampler(_device, descriptorManager, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_NEVER, 8));

	m_samplers[RenderSettings::ANISOTROPIC_X16] = std::move(Sampler(_device, descriptorManager, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP));

	this->initMainDSVandRTV();
	this->initPicking();
	this->initGeoLight();
	this->initGlowFX();
	this->initSelectionOutlines();

	m_msaaCountLastFrame = m_renderSettings.m_msaaCount;

	m_renderSettings.m_screenResolution.x = swapChain->getBackBufferDimensions().x;
	m_renderSettings.m_screenResolution.y = swapChain->getBackBufferDimensions().y;

	m_renderSurfaces.addTextureResizeMSAA(geoPassDSV);
	m_renderSurfaces.addTextureResizeMSAA(m_pickingRTVMSAA);
	m_renderSurfaces.addTextureResizeMSAA(m_glowRTVTextureMSAA);
	m_renderSurfaces.addTextureResizeMSAA(m_renderTextureMSAA);

	m_renderSurfaces.addTextureResize(m_renderTexture);
	m_renderSurfaces.addTextureResize(m_pickingRTV);
	m_renderSurfaces.addTextureResize(m_glowRTVTexture);
	m_renderSurfaces.addTextureResize(m_glowTempTexture);
}

void RendererSystem::Update()
{
	if (!mainCamera.expired())
	{
		transformAllocator->beginFrame(m_frameIndex);
		pbrMaterialAllocator->beginFrame(m_frameIndex);
		m_computeSelectionList->beginFrame(m_frameIndex);

		m_commandManager->graphicsWaitFor(m_commandManager->getCopyQueue(), m_commandManager->getCopyQueue().getLastSignalValue());

		this->passGeoLight();

		if (m_msaaCountLastFrame > 1)
		{
			this->passResolveRTVs();
		}

		if (m_renderSettings.m_enableGlow)
		{
			this->passGlowFX();
		}
		
		if (m_renderSettings.m_drawSelectionOutlines)
		{
			if (!uiSelectionList->Empty())
			{
				this->passSelectionOutlines();
			}
		}

		this->passBackBufferPrep();
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

void RendererSystem::applyRenderSettings()
{
	GraphicsPipeline::Description newDesc = m_geoLightPipeline.getDescription();
	newDesc.m_msaaSampleCount = m_renderSettings.m_msaaCount;
	m_geoLightPipeline.create(m_device, newDesc);
	m_renderSurfaces.recreateTextures(m_device, m_renderSettings.m_screenResolution.x,
		m_renderSettings.m_screenResolution.y, m_renderSettings.m_msaaCount);
}

void RendererSystem::initMainDSVandRTV()
{
	// DSV
	/*Texture::Specification specDSV;
	specDSV.m_bytesPerTexel = 4;
	specDSV.m_width = swapChain->getBackBufferDimensions().x;
	specDSV.m_height = swapChain->getBackBufferDimensions().y;
	specDSV.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	specDSV.m_dsvFormat = DXGI_FORMAT_D32_FLOAT;
	specDSV.m_usage = TEXTUREUSAGE::DSV;
	specDSV.m_sampleCount = m_renderSettings.m_msaaCount;*/

	D3D12_CLEAR_VALUE clearColorDSV;
	clearColorDSV.DepthStencil.Depth = 1;
	clearColorDSV.DepthStencil.Stencil = 0;
	clearColorDSV.Format = DXGI_FORMAT_D32_FLOAT;

	Texture::Description descDSV;
	descDSV.m_bytesPerTexel = 4;
	descDSV.m_width = swapChain->getBackBufferDimensions().x;
	descDSV.m_height = swapChain->getBackBufferDimensions().y;
	descDSV.m_initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	descDSV.m_mainFormat = DXGI_FORMAT_D32_FLOAT;
	descDSV.m_usage = DSV;
	descDSV.m_clearValue = clearColorDSV;
	descDSV.m_sampleCount = m_renderSettings.m_msaaCount;

	geoPassDSV = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descDSV));

	// RTV
	//Texture::Specification spec;
	//spec.m_bytesPerTexel = 4;
	//spec.m_width = swapChain->getBackBufferDimensions().x;
	//spec.m_height = swapChain->getBackBufferDimensions().y;
	//spec.m_initialState = D3D12_RESOURCE_STATE_COMMON;
	//spec.m_uavSrvRtvFormat = /*swapChain->getBackBufferFormat()*/DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//spec.m_usage = TEXTUREUSAGE::RTV | TEXTUREUSAGE::UAV;
	//spec.m_sampleCount = 1;

	D3D12_CLEAR_VALUE clearColor;
	clearColor.Color[0] = 0.3f;
	clearColor.Color[1] = 0.3f;
	clearColor.Color[2] = 0.3f;
	clearColor.Color[3] = 1.f;
	clearColor.Format = /*swapChain->getBackBufferFormat()*/DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	/*m_renderTexture = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, spec, clearColor));*/

	Texture::Description descRTVs;
	descRTVs.m_bytesPerTexel = 4;
	descRTVs.m_width = swapChain->getBackBufferDimensions().x;
	descRTVs.m_height = swapChain->getBackBufferDimensions().y;
	descRTVs.m_initialState = D3D12_RESOURCE_STATE_COMMON;
	descRTVs.m_mainFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	descRTVs.m_uavFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	descRTVs.m_usage = RTV | UAV;
	descRTVs.m_clearValue = clearColor;

	m_renderTexture = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descRTVs));

	descRTVs.m_sampleCount = m_renderSettings.m_msaaCount;
	m_renderTextureMSAA = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descRTVs));

	/*spec.m_sampleCount = m_renderSettings.m_msaaCount;
	m_renderTextureMSAA = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, spec, clearColor));*/

#ifdef _DEBUG
	geoPassDSV.getGPUResource()->SetName(L"geoPassDSV");
	m_renderTexture.getGPUResource()->SetName(L"m_renderTexture");
	m_renderTextureMSAA.getGPUResource()->SetName(L"m_renderTextureMSAA");
#endif // _DEBUG
}

void RendererSystem::initPicking()
{
	D3D12_CLEAR_VALUE clearColorPicking;
	clearColorPicking.Color[0] = -1.f;
	clearColorPicking.Color[1] = -1.f;
	clearColorPicking.Color[2] = -1.f;
	clearColorPicking.Color[3] = -1.f;
	clearColorPicking.Format = DXGI_FORMAT_R32_SINT;

	Texture::Description descPick;
	descPick.m_bytesPerTexel = 4;
	descPick.m_width = swapChain->getBackBufferDimensions().x;
	descPick.m_height = swapChain->getBackBufferDimensions().y;
	descPick.m_initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	descPick.m_mainFormat = DXGI_FORMAT_R32_SINT;
	descPick.m_srvFormat = DXGI_FORMAT_R32_SINT;
	descPick.m_usage = RTV | SRV;
	descPick.m_clearValue = clearColorPicking;
	descPick.m_sampleCount = 1;

	m_pickingRTV = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descPick));

	descPick.m_sampleCount = m_renderSettings.m_msaaCount;
	m_pickingRTVMSAA = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descPick));

	Buffer::Description desc;
	desc.m_heapType = D3D12_HEAP_TYPE_READBACK;
	desc.m_numElements = 1;
	desc.m_sizeBytesPerElement = 4;
	m_pickingResultBuffer = std::move(Buffer(m_device, desc, *m_trashCan));

	desc.m_heapType = D3D12_HEAP_TYPE_DEFAULT;
	m_intermPickingResultBuffer = std::move(Buffer(m_device, desc, *m_trashCan));

#ifdef _DEBUG
	m_pickingRTV.getGPUResource()->SetName(L"pickingRTV");
	m_pickingRTVMSAA.getGPUResource()->SetName(L"pickingRTVMSAA");
	m_pickingResultBuffer.getResource()->SetName(L"m_pickingResultBuffer");
	m_intermPickingResultBuffer.getResource()->SetName(L"m_intermPickingResultBuffer");
#endif // _DEBUG

	RootParameters params;
	params.addRootConstants(0, 3);
	params.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_UAV);
	m_pickingReadbackRootSig.init(m_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	ComputePipeline::Description csDesc;
	csDesc.m_rootSignature = m_pickingReadbackRootSig;
	csDesc.m_shader = m_shaderCache->loadShader("CS_PickingReadback");
	m_pickingReadbackCS = std::move(ComputePipeline(m_device, csDesc));
}

void RendererSystem::initGeoLight()
{
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
	paramsPBR.addRootConstants(6, 2, D3D12_SHADER_VISIBILITY_PIXEL); // 11 pass constants

	pbrRootSig.init(m_device, &paramsPBR, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	InputLayout layout;
	layout.addElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
	layout.addElement("UV", DXGI_FORMAT_R32G32_FLOAT);
	layout.addElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
	layout.addElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);

	GraphicsPipeline::Description desc;
	desc.m_depthStencilFormat = geoPassDSV.getMainFormat();
	desc.m_inputLayout = layout;
	desc.m_msaaSampleCount = m_renderSettings.m_msaaCount;
	desc.m_rasterizerDesc = solidRaster.getDesc();
	desc.m_renderTargetFormats = { m_renderTexture.getMainFormat(), DXGI_FORMAT_R32_SINT, swapChain->getBackBufferFormat()};
	desc.m_shaders.m_vertexShader = m_shaderCache->loadShader("VS_PBR");
	desc.m_shaders.m_pixelShader = m_shaderCache->loadShader("PS_PBR");
	desc.m_rootSignature = pbrRootSig;
	m_geoLightPipeline = std::move(GraphicsPipeline(m_device, desc));
}

void RendererSystem::initGlowFX()
{
	RootParameters params;
	params.addRootConstants(0, 3);
	m_processGlowRootSig.init(m_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	ComputePipeline::Description desc;
	desc.m_rootSignature = m_processGlowRootSig;
	desc.m_shader = m_shaderCache->loadShader("CS_GlowHorizontal");
	m_computeGlowHorizontal = std::move(ComputePipeline(m_device, desc));

	desc.m_shader = m_shaderCache->loadShader("CS_GlowVertical");
	m_computeGlowVertical = std::move(ComputePipeline(m_device, desc));


	D3D12_CLEAR_VALUE clearColorPicking;
	clearColorPicking.Color[0] = -1.f;
	clearColorPicking.Color[1] = -1.f;
	clearColorPicking.Color[2] = -1.f;
	clearColorPicking.Color[3] = -1.f;
	clearColorPicking.Format = DXGI_FORMAT_R32_SINT;

	Texture::Description descPick;
	descPick.m_bytesPerTexel = 4;
	descPick.m_width = swapChain->getBackBufferDimensions().x;
	descPick.m_height = swapChain->getBackBufferDimensions().y;
	descPick.m_initialState = D3D12_RESOURCE_STATE_COMMON;
	descPick.m_mainFormat = DXGI_FORMAT_R32_SINT;
	descPick.m_srvFormat = DXGI_FORMAT_R32_SINT;
	descPick.m_usage = RTV | SRV;
	descPick.m_clearValue = clearColorPicking;
	descPick.m_sampleCount = 1;

	m_pickingRTV = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, descPick));


	Texture::Description specGlow;
	specGlow.m_bytesPerTexel = 4;
	specGlow.m_width = swapChain->getBackBufferDimensions().x;
	specGlow.m_height = swapChain->getBackBufferDimensions().y;
	specGlow.m_initialState = D3D12_RESOURCE_STATE_COMMON;
	specGlow.m_mainFormat = swapChain->getBackBufferFormat();
	specGlow.m_srvFormat = swapChain->getBackBufferFormat();
	specGlow.m_uavFormat = swapChain->getBackBufferFormat();
	specGlow.m_usage = TEXTUREUSAGE::RTV | TEXTUREUSAGE::UAV | TEXTUREUSAGE::SRV;
	specGlow.m_sampleCount = 1;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Color[0] = 0.f;
	clearValue.Color[1] = 0.f;
	clearValue.Color[2] = 0.f;
	clearValue.Color[3] = 0.f;
	clearValue.Format = swapChain->getBackBufferFormat();
	specGlow.m_clearValue = clearValue;

	m_glowTempTexture = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, specGlow));
	m_glowRTVTexture = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, specGlow));

	specGlow.m_sampleCount = m_renderSettings.m_msaaCount;
	m_glowRTVTextureMSAA = std::move(Texture(m_device, *m_descriptorManager, *m_trashCan, specGlow));

#ifdef _DEBUG
	m_glowTempTexture.getGPUResource()->SetName(L"m_glowTempTexture");
	m_glowRTVTexture.getGPUResource()->SetName(L"m_glowRTVTexture");
	m_glowRTVTextureMSAA.getGPUResource()->SetName(L"m_glowRTVTextureMSAA");
#endif // DEBUG
}

void RendererSystem::initSelectionOutlines()
{
	RootParameters params;
	params.addRootConstants(0, 11);
	params.addRootDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_SRV);
	m_selectionRootSig.init(m_device, &params, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, 0, nullptr);

	ComputePipeline::Description desc;
	desc.m_rootSignature = m_selectionRootSig;
	desc.m_shader = m_shaderCache->loadShader("CS_FXOutline");
	m_selectionPipeState = std::move(ComputePipeline(m_device, desc));
}

void RendererSystem::passGeoLight()
{
	GraphicsContextHandle renderContext = m_commandManager->getGraphicsContext();

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	renderContext.setDescriptorHeaps(2, heap);

	renderContext.clearDepthStencilView(geoPassDSV);

	if (m_msaaCountLastFrame > 1)
	{
		renderContext.transitionTexture(m_pickingRTVMSAA, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_pickingRTVMSAA);

		renderContext.transitionTexture(m_renderTextureMSAA, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_renderTextureMSAA);

		renderContext.transitionTexture(m_glowRTVTextureMSAA, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_glowRTVTextureMSAA);

		const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = { m_renderTextureMSAA.getRTVHandle().getCPUHandle(),
			m_pickingRTVMSAA.getRTVHandle().getCPUHandle(), m_glowRTVTextureMSAA.getRTVHandle().getCPUHandle() };
		D3D12_CPU_DESCRIPTOR_HANDLE handle = geoPassDSV.getDSVHandle().getCPUHandle();
		renderContext.setOMRenderTargets(3, rtvHandles, false, &handle);
	}
	else
	{
		renderContext.transitionTexture(m_pickingRTV, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_pickingRTV);

		renderContext.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_renderTexture);

		renderContext.transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		renderContext.clearRenderTargetView(m_glowRTVTexture);

		const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[3] = { m_renderTexture.getRTVHandle().getCPUHandle(),
			m_pickingRTV.getRTVHandle().getCPUHandle(), m_glowRTVTexture.getRTVHandle().getCPUHandle() };

		D3D12_CPU_DESCRIPTOR_HANDLE handle = geoPassDSV.getDSVHandle().getCPUHandle();
		renderContext.setOMRenderTargets(3, rtvHandles, false, &handle);
	}

	renderContext.setIAPrimiteTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	renderContext.setRSScizzorRects(1, &swapChain->getScissorRect());

	renderContext.setRSViewports(1, &swapChain->getViewPort());

	m_geoLightPipeline.setPipeline(renderContext);

	{
		std::shared_ptr<Camera> cam = mainCamera.lock();
		renderContext.setGraphicsConstantBufferView(1, cam->GetBuffer()->getVirtualAddress());
		renderContext.setGraphics32BitRootConstants(2, 4, (void*)&cam->GetForward(), 0);
		renderContext.setGraphics32BitRootConstants(2, 4, (void*)&cam->GetPosition(), 4);
	}

	renderContext.setGraphicsConstantBufferView(3, lManager->numLightsCB->getVirtualAddress());

	renderContext.setGraphicsShaderResourceView(4, lManager->pLightList.dataBuffer.getVirtualAddress());

	renderContext.setGraphics32BitRootConstants(5, 8, (void*)&lManager->dLightData.direction, 0);

	struct PassConstants
	{
		int samplerIndex;
		float hdrExposure;
	};

	PassConstants passConstants{ m_samplers.at(m_renderSettings.m_currentSampler).getHandle().getHeapIndex() , m_renderSettings.m_hdrExposure };
	renderContext.setGraphics32BitRootConstants(11, 2, &passConstants, 0);

	renderContext.setGraphics32BitRootConstant(7, shadowMap.getSRVHandle().getHeapIndex(), 0); // bug with 0 as offset?

	renderContext.setGraphics32BitRootConstants(8, 16, (void*)&lManager->dLightData.VPMatrix, 0); // bug with 0 as offset?

	renderContext.setGraphicsShaderResourceView(10, transformAllocator->getVirtualAddress());

	renderContext.setGraphicsShaderResourceView(6, pbrMaterialAllocator->getVirtualAddress());
	

	PBRMaterial* const defualtMat = mManager->GetMaterial<PBRMaterial>("DefaultPBRMaterial");

	std::vector<aZeroECS::Entity>& entities = m_entities.GetObjects();
	int numEntities = entities.size();

	aZeroAlloc::Allocation<PBRMaterialInfo> pbrMatAlloc = pbrMaterialAllocator->getAllocation(numEntities);
	int defaultMatIndex = pbrMatAlloc.currentOffset();
	*pbrMatAlloc = defualtMat->GetInfo();
	++pbrMatAlloc;

	//
	aZeroAlloc::Allocation<DXM::Matrix> transformAlloc = transformAllocator->getAllocation(numEntities);
	transformAllocator->updateAll(renderContext.getList(), m_frameIndex);

	for (int i = 0; i < numEntities; i++)
	{
		PixelDrawConstantsPBR perDraw;

		PBRMaterial* const mat = mManager->GetMaterial<PBRMaterial>(m_componentManager.GetComponent<MaterialComponent>(entities[i])->materialID);

		// Transform
		Transform* const tf = m_componentManager.GetComponent<Transform>(entities[i]);
		parentSystem->CalculateWorld(entities[i], tf);

		*transformAlloc = tf->GetWorldMatrix();

		tf->m_frameResourceIndex = transformAlloc.currentOffset();
		renderContext.setGraphics32BitRootConstant(0, tf->m_frameResourceIndex, 0);

		// Material
		// Reuse allocated material index
		if (!mat)
			perDraw.materialIndex = defaultMatIndex;
		else
		{
			perDraw.materialIndex = pbrMatAlloc.currentOffset();
			PBRMaterialInfo info = mat->GetInfo();

			if (!m_renderSettings.m_enableGlow)
				info.enableGlow = false;

			*pbrMatAlloc = info;
			++pbrMatAlloc;
		}

		// Mesh
		perDraw.receiveShadows = m_componentManager.GetComponent<Mesh>(entities[i])->receiveShadows;

		// Other
		perDraw.pickingID = entities[i].m_id;

		renderContext.setGraphics32BitRootConstants(9, 3, (void*)&perDraw, 0);

		ModelAsset* model = modelCache->GetResource(m_componentManager.GetComponent<Mesh>(entities[i])->GetID());
		D3D12_VERTEX_BUFFER_VIEW vbView = model->getVertexBufferView();
		renderContext.setIAVertexBuffers(0, 1, &vbView);
		D3D12_INDEX_BUFFER_VIEW ibView = model->getIndexBufferView();
		renderContext.setIAIndexBuffer(&ibView);

		renderContext.drawIndexedInstanced(model->getNumIndices(), 1, 0, 0, 0);

		++transformAlloc;
	}

	m_commandManager->executeContext(renderContext);
}

void RendererSystem::passResolveRTVs()
{
	GraphicsContextHandle resolveContext = m_commandManager->getGraphicsContext();

	resolveContext.transitionTexture(m_renderTextureMSAA, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	resolveContext.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	resolveContext.resolveSubresource(m_renderTexture, 0, m_renderTextureMSAA, 0);

	resolveContext.transitionTexture(m_glowRTVTextureMSAA, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	resolveContext.transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
	resolveContext.resolveSubresource(m_glowRTVTexture, 0, m_glowRTVTextureMSAA, 0);

	m_commandManager->executeContext(resolveContext);
}

void RendererSystem::passGlowFX()
{
	{
		GraphicsContextHandle glowPrepContext = m_commandManager->getGraphicsContext();

		glowPrepContext.transitionTexture(m_glowRTVTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		glowPrepContext.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_commandManager->executeContext(glowPrepContext);
	}

	m_commandManager->computeWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());

	ComputeContextHandle context = m_commandManager->getComputeContext();
	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	context.setDescriptorHeaps(2, heap);

	m_computeGlowHorizontal.setPipeline(context);

	context.transitionTexture(m_glowTempTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	struct FxBlurData
	{
		int srcTextureID;
		int dstTextureID;
		int kernelRadius;
	};
	FxBlurData data;

	// src is glow texture
	// dst is empty glow result texture
	data.srcTextureID = m_glowRTVTexture.getSRVHandle().getHeapIndex();
	data.dstTextureID = m_glowTempTexture.getUAVHandle().getHeapIndex();
	data.kernelRadius = m_renderSettings.m_glowRadius;

	context.setCompute32BitRootConstants(0, 3, &data, 0);

	context.dispatch(std::ceil(m_renderTexture.getWidth() / 16.f), std::ceil(m_renderTexture.getHeight() / 16.f), 1);

	context.transitionTexture(m_glowTempTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// src is blur pass 1/2 horizontal texture
	// dst is frame color rtv
	data.srcTextureID = m_glowTempTexture.getSRVHandle().getHeapIndex();
	data.dstTextureID = m_renderTexture.getUAVHandle().getHeapIndex();

	m_computeGlowVertical.setPipeline(context);

	context.setCompute32BitRootConstants(0, 3, &data, 0);

	// needs m_renderTexture in uav mode
	// needs m_blurTempTexture in srv/generic-read mode
	context.dispatch(std::ceil(m_renderTexture.getWidth() / 16.f), std::ceil(m_renderTexture.getHeight() / 16.f), 1);

	m_commandManager->executeContext(context);
}

void RendererSystem::passSelectionOutlines()
{
	{
		GraphicsContextHandle outlinePrepContext = m_commandManager->getGraphicsContext();

		outlinePrepContext.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if(m_msaaCountLastFrame > 1)
			outlinePrepContext.transitionTexture(m_pickingRTVMSAA, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		else
			outlinePrepContext.transitionTexture(m_pickingRTV, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		m_commandManager->executeContext(outlinePrepContext);
	}

	m_commandManager->copyWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());
	m_commandManager->computeWaitFor(m_commandManager->getGraphicsQueue(), m_commandManager->getGraphicsQueue().getLastSignalValue());

	ComputeContextHandle context = m_commandManager->getComputeContext();
	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	context.setDescriptorHeaps(2, heap);

	m_selectionPipeState.setPipeline(context);

	struct OutlinePassData
	{
		DXM::Vector3 mainOutlineColor = { 1,0,0 };
		int outlineThickness = 2;
		DXM::Vector3 secondOutlineColor = { 1,1,0 };
		int renderTargetID = 0;
		int pickingTextureID = 0;
		int numSelections = -1;
		int msaaOn = 0;
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
	data.mainOutlineColor = m_renderSettings.m_mainOutlineColor;
	data.secondOutlineColor = m_renderSettings.m_secondaryOutlineColor;
	data.outlineThickness = m_renderSettings.m_outlineThickness;
	data.msaaOn = m_msaaCountLastFrame > 1 ? 1 : 0;

	data.renderTargetID = m_renderTexture.getUAVHandle().getHeapIndex();

	if(m_msaaCountLastFrame > 1)
		data.pickingTextureID = m_pickingRTVMSAA.getSRVHandle().getHeapIndex();
	else
		data.pickingTextureID = m_pickingRTV.getSRVHandle().getHeapIndex();

	context.setCompute32BitRootConstants(0, 11, &data, 0);
	context.setComputeShaderResourceView(1, m_computeSelectionList->getVirtualAddress());
	context.dispatch(std::ceil(m_renderTexture.getWidth() / 16.f), std::ceil(m_renderTexture.getHeight() / 16.f), 1);

	m_commandManager->executeContext(context);
}

void RendererSystem::passBackBufferPrep()
{
	m_commandManager->graphicsWaitFor(m_commandManager->getComputeQueue(), m_commandManager->getComputeQueue().getLastSignalValue());

	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	context.transitionTexture(m_renderTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
	context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
	context.copyResource(currentBackBuffer->getGPUResource(), m_renderTexture.getGPUResource());

	m_commandManager->executeContext(context);
}

void RendererSystem::resolveMSAAPicking(int xTexel, int yTexel)
{
	m_commandManager->graphicsWaitFor(m_commandManager->getComputeQueue(), m_commandManager->getComputeQueue().getLastSignalValue());
	GraphicsContextHandle context = m_commandManager->getGraphicsContext();

	ID3D12DescriptorHeap* const heap[] = { m_descriptorManager->getResourceHeap(), m_descriptorManager->getSamplerHeap() };
	context.setDescriptorHeaps(2, heap);

	m_pickingReadbackCS.setPipeline(context);

	struct PickingData
	{
		int m_pickingTextureIndex;
		int m_xTexel;
		int m_yTexel;
	};
	PickingData rdBack;

	context.transitionTexture(m_pickingRTVMSAA, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	rdBack.m_pickingTextureIndex = m_pickingRTVMSAA.getSRVHandle().getHeapIndex();
	
	rdBack.m_xTexel = xTexel;
	rdBack.m_yTexel = yTexel;
	context.setCompute32BitRootConstants(0, 3, &rdBack, 0);
	context.setComputeUnorderedAccessView(1, m_intermPickingResultBuffer.getVirtualAddress());
	context.dispatch(1, 1, 1);

	context.transitionTexture(m_pickingRTVMSAA, D3D12_RESOURCE_STATE_RENDER_TARGET);

	context.copyBufferRegion(m_pickingResultBuffer.getResource(), 0, m_intermPickingResultBuffer.getResource(), 0, m_pickingResultBuffer.getTotalSizeBytes());

	m_commandManager->executeContext(context);
}
