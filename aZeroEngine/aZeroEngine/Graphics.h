#pragma once
#include <iostream>
#include "SwapChain.h"
#include "ShaderDescriptorHeap.h"
#include "HiddenDescriptorHeap.h"
#include "DepthStencil.h"
#include "VertexBufferCache.h"
#include "MaterialManager.h"
#include "RenderSystem.h"
#include "ResourceManager.h"

// TESTING
#include "ConstantBuffer.h"
#include "VertexDefinitions.h"

#include "Scene.h"
#include "RenderSystem.h"

class Graphics
{
private:
	float clearColor[4] = { 0.7f, 0.7f, 0.7f, 1 };
public:
	Graphics(AppWindow* _window, HINSTANCE _instance);
	~Graphics();
	void Initialize(AppWindow* _window, HINSTANCE _instance);
	void Begin();
	void Render(AppWindow* _window);
	void Present();

	void WaitForGPU();

	ID3D12Device* device;
	CommandQueue* directCommandQueue;

	HiddenDescriptorHeap* rtvHeap;
	HiddenDescriptorHeap* dsvHeap;

	// temp
	CommandList directCmdList;
	CommandAllocator* allocator;

	// Presenting and culling
	SwapChain* swapChain;
	RenderTarget* currentBackBuffer;
	int nextSyncSignal = 0;
	int frameIndex;
	int frameCount;

	VertexBufferCache* vbCache;
	Texture2DCache* textureCache;
	ResourceManager resourceManager;
	MaterialManager materialManager;
	LightManager* lManager;

	Scene* scene = nullptr;
	ECS* ecs;

	BasicRendererSystem* renderSystem;
};