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
	PipelineState phongPso;
	RootSignature phongRootSig;

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
	bool pbr = true;

public:
	void SetBackBuffer(RenderTarget* _currentBackBuffer) { currentBackBuffer = _currentBackBuffer; }

	void SetMainCameraGeo(std::weak_ptr<Camera> _camera) { mainCamera = _camera; }

	std::shared_ptr<Camera> GetMainCameraGeo() { return mainCamera.lock(); }


	RendererSystem() = default;

	RendererSystem(ComponentManager& _componentManager)
		:ECSystem(_componentManager)
	{

	}
	
	void Init(ID3D12Device* _device,
		ResourceEngine* _resourceEngine,
		VertexBufferCache* _vbCache,
		std::shared_ptr<LightManager> _lManager,
		MaterialManager* _mManager,
		SwapChain& _swapChain, HINSTANCE _instance, HWND _winHandle)
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
		swapChain = &_swapChain;

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

	~RendererSystem()
	{
	}

	// Inherited via ECSystem
	virtual void Update() override
	{
		if (!mainCamera.expired())
		{
			ShadowPassBegin();
			GeometryPass();
		}
	}
};