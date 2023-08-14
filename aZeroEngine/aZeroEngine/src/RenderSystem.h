#pragma once
#include "Sampler.h"
#include "AppWindow.h"
#include "Camera.h"
#include "LinearAllocator.h"
#include "ParentSystem.h"
#include "Scene.h"
#include "SelectionList.h"
#include "Texture.h"
#include "ResourceRecycler.h"
#include "CommandManager.h"
#include "ECS.h"
#include "ComputePipeline.h"
#include "RenderSurfaces.h"
#include "RasterState.h"

#include "GraphicsMemory.h"
#include "DebugDraw.h"
#include "EffectPipelineStateDescription.h"
#include "Effects.h"
#include "CommonStates.h"
#include <array>
#include "GraphicsPipeline.h"
#include "ShaderCache.h"

class Scene;

/** @brief Handles the rendering of bound Entity objects.
* Uses dependency injection to access neccessary engine objects.
*/
class RendererSystem : public aZeroECS::System
{
public:
	// Render Settings
	struct RenderSettings
	{
		// Needs a notifyRenderSettingsUpdate() call to be applied
		int m_msaaCount = 1;
		DXM::Vector2 m_screenResolution = DXM::Vector2(1920, 1080);
		//

		// Outline Settings
		bool m_drawSelectionOutlines = true;
		int m_outlineThickness = 2;
		DXM::Vector3 m_mainOutlineColor = DXM::Vector3(1, 0, 0);
		DXM::Vector3 m_secondaryOutlineColor = DXM::Vector3(1, 1, 0);

		// Glow Settings
		bool m_enableGlow = true;
		int m_glowRadius = 20;

		// Sampler Settings
		enum SAMPLERTYPE { POINT, LINEAR, ANISOTROPIC_X8, ANISOTROPIC_X16, END };
		SAMPLERTYPE m_currentSampler = SAMPLERTYPE::ANISOTROPIC_X16;

		// Gamma and hdr
		float m_gamma = 2.2f;
		float m_hdrExposure = 1.5f;
	};

private:

	UINT m_frameIndex = 0;

	// Dependency Injection
	ModelCache* modelCache = nullptr;
	std::shared_ptr<LightManager> lManager = nullptr;
	MaterialManager* mManager = nullptr;
	CommandManager* m_commandManager = nullptr;
	DescriptorManager* m_descriptorManager = nullptr;
	ResourceRecycler* m_trashCan = nullptr;
	ID3D12Device* m_device = nullptr;
	ShaderCache* m_shaderCache = nullptr;

	Texture shadowMap;
	D3D12_VIEWPORT lightViewPort;
	D3D12_RECT lightScizzorRect;

	SwapChain* swapChain = nullptr;


	// Geo / Light pass
	GraphicsPipeline m_geoLightPipeline;
	RootSignature pbrRootSig;

	RootSignature m_selectionRootSig;
	ComputePipeline m_selectionPipeState;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<int>> m_computeSelectionList;

	// Shared Resources
	RasterState solidRaster;

	std::array<Sampler, RenderSettings::END> m_samplers;

	void initShadowPass(ID3D12Device* _device);
	void shadowPassBegin();

	// Picking
	ComputePipeline m_pickingReadbackCS;
	RootSignature m_pickingReadbackRootSig;

	Texture m_pickingRTV;
	Texture m_pickingRTVMSAA;
	Buffer m_pickingResultBuffer;
	Buffer m_intermPickingResultBuffer;
	
	// FX Glow
	Texture m_glowRTVTexture;
	Texture m_glowRTVTextureMSAA;
	Texture m_glowTempTexture;
	RootSignature m_processGlowRootSig;
	ComputePipeline m_computeGlowHorizontal;
	ComputePipeline m_computeGlowVertical;

	struct PixelDrawConstantsPBR
	{
		int receiveShadows = 1;
		int materialIndex = 0;
		int pickingID = 0;
	};

	// Render Surfaces
	RenderSurfaces m_renderSurfaces;
	Texture* currentBackBuffer = nullptr;
	Texture m_renderTexture;
	Texture m_renderTextureMSAA;
	Texture geoPassDSV;

	// Render Settings
	bool m_renderSettingsDirty = false;
	UINT m_msaaCountLastFrame = 0;

	std::unique_ptr<aZeroAlloc::LinearDefaultAllocator<DXM::Matrix>> transformAllocator;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<PBRMaterialInfo>> pbrMaterialAllocator;

private:
	// Inits
	void initMainDSVandRTV();
	void initPicking();
	void initGeoLight();
	void initGlowFX();
	void initSelectionOutlines();

	// Passes
	void passGeoLight();
	void passResolveRTVs();
	void passGlowFX();
	void passSelectionOutlines();
	void passBackBufferPrep();

	void applyRenderSettings();
	void resolveMSAAPicking(int xTexel, int yTexel);

public:

	RenderSettings m_renderSettings;

	ParentSystem* parentSystem = nullptr;
	std::weak_ptr<Camera> mainCamera;

	SelectionList* uiSelectionList = nullptr;
	Scene* currentScene = nullptr;

	RendererSystem() = default;

	RendererSystem(aZeroECS::ComponentManager& _componentManager, ID3D12Device* _device, 
		CommandManager& commandManager, ResourceRecycler& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
		std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, ShaderCache* shaderCache, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle)
		:aZeroECS::System(_componentManager)
	{ 
		this->Init(_device, commandManager, trashcan, descriptorManager, _modelCache, _lManager, _mManager, shaderCache, _swapChain, _instance, _winHandle);
	}
	
	void Init(ID3D12Device* _device, CommandManager& commandManager, ResourceRecycler& trashcan, DescriptorManager& descriptorManager, ModelCache* _modelCache,
		std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, ShaderCache* shaderCache, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle);

	~RendererSystem() = default;

	void BeginFrame(Texture* currentBackBuffer, UINT frameIndex)
	{
		this->currentBackBuffer = currentBackBuffer;
		m_frameIndex = frameIndex;
	}

	// Inherited via ECSystem
	virtual void Update() override;

	void endFrame()
	{
		if (m_renderSettingsDirty)
		{
			m_commandManager->flushCPUGraphics();
			m_commandManager->flushCPUCompute();
			m_commandManager->flushCPUCopy();
			this->applyRenderSettings();
			m_renderSettingsDirty = false;
		}

		m_msaaCountLastFrame = m_renderSettings.m_msaaCount;
	}

	void notifyRenderSettingsUpdate() { m_renderSettingsDirty = true; };

	int GetPickingID(int _xPos, int _yPos)
	{
		// Return early if _xPos and _yPos are bigger than swapchain width and height
		if (_xPos >= m_pickingRTV.getWidth() || _yPos >= m_pickingRTV.getHeight()
			|| _xPos < 0 || _yPos < 0)
			return -1;

		int index = _xPos * m_pickingRTV.getBytesPerTexel() + _yPos * m_pickingRTV.getPaddedRowPitch();

		int data = 0;

		if (m_renderSettings.m_msaaCount > 1)
		{
			this->resolveMSAAPicking(_xPos, _yPos);
		}
		else
		{
			GraphicsContextHandle context = m_commandManager->getGraphicsContext();

			context.transitionTexture(m_pickingRTV, D3D12_RESOURCE_STATE_COPY_SOURCE);

			context.copyTextureToBuffer(m_device, m_pickingRTV, m_pickingResultBuffer,
					{ static_cast<float>(_xPos), static_cast<float>(_yPos) }, 0, { 1,1 });
			OutputDebugString(L"YO\n");

			context.transitionTexture(m_pickingRTV, D3D12_RESOURCE_STATE_RENDER_TARGET);

			m_commandManager->executeContext(context);
		}

		m_commandManager->flushCPUGraphics();

		m_pickingResultBuffer.readbackData(data, 0);

		return data;
	}
};