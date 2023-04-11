#pragma once
#include "ECSBase.h"
#include "Texture2DCache.h"
#include "DescriptorManager.h"
#include "MaterialManager.h"
#include "VertexBufferCache.h"
#include "Sampler.h"
#include "PipelineState.h"
#include "LightManager.h"
#include "AppWindow.h"
#include "Camera.h"

/** @brief Handles the rendering of bound Entity objects.
* Uses dependency injection to access neccessary engine objects.
*/
class RendererSystem : public ECSystem
{
private:

	// Dependency Injection
	ResourceEngine* resourceEngine = nullptr;
	VertexBufferCache* vbCache = nullptr;
	std::shared_ptr<LightManager> lManager = nullptr;
	MaterialManager* mManager = nullptr;

	// Shadow Pass
	PipelineState shadowPso;
	RootSignature shadowRootSig;

	DepthStencil shadowMap;
	D3D12_VIEWPORT lightViewPort;
	D3D12_RECT lightScizzorRect;

	SwapChain* swapChain = nullptr;

	// Geometry Pass
	DepthStencil geoPassDSV;

	PipelineState pbrPso;
	RootSignature pbrRootSig;

	// Shared Resources
	InputLayout layout;

	RasterState solidRaster;
	RasterState wireFrameRaster;

	Sampler anisotropicWrapSampler;
	Sampler anisotropicBorderSampler;

	RenderTarget* currentBackBuffer = nullptr;

	void InitShadowPass(ID3D12Device* _device);
	void ShadowPassBegin();

	void InitGeometryPass(ID3D12Device* _device);
	void GeometryPass();
	
	std::weak_ptr<Camera> mainCamera;

public:
	void SetBackBuffer(RenderTarget* _currentBackBuffer) { currentBackBuffer = _currentBackBuffer; }

	void SetMainCameraGeo(std::weak_ptr<Camera> _camera) { mainCamera = _camera; }

	std::shared_ptr<Camera> GetMainCameraGeo() { return mainCamera.lock(); }


	RendererSystem() = default;

	RendererSystem(ComponentManager& _componentManager) :ECSystem(_componentManager) { }
	
	void Init(ID3D12Device* _device, ResourceEngine* _resourceEngine, VertexBufferCache* _vbCache,
		std::shared_ptr<LightManager> _lManager, MaterialManager* _mManager, SwapChain* _swapChain, HINSTANCE _instance, HWND _winHandle);

	~RendererSystem() = default;

	// Inherited via ECSystem
	virtual void Update() override;
};