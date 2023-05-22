#pragma once
#include "Sampler.h"
#include "PipelineState.h"
#include "AppWindow.h"
#include "Camera.h"
#include "LinearAllocator.h"
#include "ParentSystem.h"
#include "Scene.h"
#include "SelectionList.h"
#include "Texture.h"
#include "ReadbackBuffer.h"
#include "ResourceTrashcan.h"
#include "CommandManager.h"
#include "ECS.h"
#include "ComputePipelineState.h"

class Scene;

/** @brief Handles the rendering of bound Entity objects.
* Uses dependency injection to access neccessary engine objects.
*/
class RendererSystem : public aZeroECS::System
{
private:

	UINT m_frameIndex = 0;

	// Dependency Injection
	ModelCache* modelCache = nullptr;
	std::shared_ptr<LightManager> lManager = nullptr;

	// !!!!
	MaterialManager* mManager = nullptr;
	CommandManager* m_commandManager = nullptr;
	DescriptorManager* m_descriptorManager = nullptr;
	ResourceTrashcan* m_trashCan = nullptr;
	ID3D12Device* m_device = nullptr;

	// Shadow Pass
	PipelineState shadowPso;
	RootSignature shadowRootSig;

	Texture shadowMap;
	D3D12_VIEWPORT lightViewPort;
	D3D12_RECT lightScizzorRect;

	SwapChain* swapChain = nullptr;

	// Geometry Pass
	Texture geoPassDSV;

	// Picking
	std::shared_ptr<Texture> pickingRTV;
	ReadbackBuffer m_readbackBuffer;

	// Outline
	PipelineState outlinePSO;
	RootSignature outlineRoot;

	PipelineState pbrPso;
	PipelineState pbrPsoNoPrepass;
	RootSignature pbrRootSig;

	PipelineState m_pbrPsoTransp;
	RootSignature m_pbrRootSigTransp;

	RootSignature m_csRootSig;
	ComputePipelineState* m_csPipeState;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<int>> m_computeSelectionList;

	// Z Pre Pass
	PipelineState zPassPSO;
	RootSignature zPassRootSig;

	// Shared Resources
	RasterState solidRaster;
	RasterState wireFrameRaster;

	Sampler anisotropicWrapSampler;
	Sampler anisotropicBorderSampler;

	Texture* currentBackBuffer = nullptr;

	void initShadowPass(ID3D12Device* _device);
	void shadowPassBegin();

	void initGeometryPass(ID3D12Device* _device);
	void passesNoPrepass();
	void passesIncPrepass();

	void initZPrePass(ID3D12Device* _device);
	
	// FX Blending
	Texture m_glowRTVTexture;
	Texture m_glowTempTexture;
	RootSignature m_processGlowRootSig;
	ComputePipelineState* m_processGlowPipeState;
	void initFXProcessGlow();
	void fxProcessGlow();
	
	void postEffectOutlinePass();
	void prepareBackbuffer();
	
	std::weak_ptr<Camera> mainCamera;

	struct PixelDrawConstantsPBR
	{
		int receiveShadows = 1;
		int materialIndex = 0;
		int pickingID = 0;
	};

public:

	std::unique_ptr<aZeroAlloc::LinearDefaultAllocator<DXM::Matrix>> transformAllocator;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<PBRMaterialInfo>> pbrMaterialAllocator;

	ParentSystem* parentSystem = nullptr;
	Texture m_renderTexture;

	// Pass Settings
	bool m_prePassUsed = true;
	bool m_drawSelectionOutlines = true;

	// Blur Settings
	bool m_blurByDistance = true;
	int m_blurDistRadiusX = 20;
	int m_blurDistRadiusY = 20;

	int m_outlineThickness = 2;
	DXM::Vector3 m_mainOutlineColor = DXM::Vector3(1, 0, 0);
	DXM::Vector3 m_secondaryOutlineColor = DXM::Vector3(1, 1, 0);

	void SetMainCameraGeo(std::weak_ptr<Camera> _camera) { mainCamera = _camera; }

	std::shared_ptr<Camera> GetMainCameraGeo() { return mainCamera.lock(); }

	SelectionList* uiSelectionList = nullptr;
	Scene* currentScene = nullptr;

	RendererSystem() = default;

	RendererSystem(aZeroECS::ComponentManager& _componentManager, ID3D12Device* _device, 
		CommandManager& commandManager, ResourceTrashcan& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
		std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
		:aZeroECS::System(_componentManager)
	{ 
		this->Init(_device, commandManager, trashcan, descriptorManager, _modelCache, _lManager, _mManager, _swapChain, _instance, _winHandle);
	}
	
	void Init(ID3D12Device* _device, CommandManager& commandManager, ResourceTrashcan& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
		std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle);

	~RendererSystem() = default;

	void BeginFrame(Texture* currentBackBuffer, UINT frameIndex)
	{
		this->currentBackBuffer = currentBackBuffer;
		m_frameIndex = frameIndex;
	}
	// Inherited via ECSystem
	virtual void Update() override;

	int GetPickingID(int _xPos, int _yPos)
	{
		// Return early if _xPos and _yPos are bigger than swapchain width and height
		if (_xPos > pickingRTV->getDimensions().x || _yPos > pickingRTV->getDimensions().y
			|| _xPos < 0 || _yPos < 0)
			return -1;

		int index = _xPos * pickingRTV->getBytesPerTexel() + _yPos * pickingRTV->getPaddedRowPitch();

		int data = 0;

		{
			GraphicsContextHandle context = m_commandManager->getGraphicsContext();

			context.transitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_COPY_SOURCE);
			context.copyTextureToBuffer(m_device, pickingRTV->getGPUOnlyResource(), m_readbackBuffer.getResource(), 0, 0, 0, nullptr);
			context.transitionTexture(*pickingRTV, D3D12_RESOURCE_STATE_RENDER_TARGET);

			m_commandManager->executeContext(context);
		}

		m_commandManager->flushCPUGraphics();

		memcpy(&data, (char*)m_readbackBuffer.getMappedPointer() + index, pickingRTV->getBytesPerTexel());

		return data;
	}
};