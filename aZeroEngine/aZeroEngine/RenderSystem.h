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
	RootSignature pbrRootSig;

	RootSignature m_csRootSig;
	ComputePipelineState* m_csPipeState;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<int>> m_computeSelectionList;

	// Shared Resources
	InputLayout layout;

	RasterState solidRaster;
	RasterState wireFrameRaster;

	Sampler anisotropicWrapSampler;
	Sampler anisotropicBorderSampler;

	Texture* currentBackBuffer = nullptr;
	Texture m_renderTexture;

	void InitShadowPass(ID3D12Device* _device);
	void ShadowPassBegin();

	void InitGeometryPass(ID3D12Device* _device);
	void GeometryPass();

	void PostEffectOutlinePass();
	void PrepareBackbuffer();
	
	std::weak_ptr<Camera> mainCamera;

	struct PixelDrawConstantsPBR
	{
		int receiveShadows = 1;
		int materialIndex = 0;
		int pickingID = 0;
	};

public:

	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<DXM::Matrix>> transformAllocator;
	std::unique_ptr<aZeroAlloc::LinearUploadAllocator<PBRMaterialInfo>> pbrMaterialAllocator;

	ParentSystem* parentSystem = nullptr;

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

		memcpy(&data, (char*)m_readbackBuffer.getMappedPointer() + index, 4);

		return data;
	}
};