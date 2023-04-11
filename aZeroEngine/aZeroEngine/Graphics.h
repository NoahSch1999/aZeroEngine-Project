#pragma once
#include <iostream>
#include "SwapChain.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "UserInterface.h"
#include "ResourceEngine.h"
#include "LightSystem.h"
#include "PickingSystem.h"

class Graphics
{
private:
	std::unordered_map<std::string, std::shared_ptr<UserInterface>>userInterfaces;
public:
	float clearColor[4] = { 0.7f, 0.7f, 0.7f, 1 };
	Graphics(AppWindow& _window, HINSTANCE _instance);
	~Graphics();
	void Initialize(AppWindow& _window, HINSTANCE _instance);
	void BeginFrame();
	void Render(AppWindow* _window);
	void EndFrame();

	ID3D12Device* device;
	AppWindow& window;
	std::shared_ptr<SwapChain>swapChain;

	RenderTarget* currentBackBuffer;
	int nextSyncSignal = 0;
	int frameIndex = 0;
	int frameCount = 0;

	ECS ecs;
	ResourceEngine resourceEngine;

	DescriptorManager descriptorManager;
	MaterialManager materialManager;

	VertexBufferCache vbCache;
	Texture2DCache textureCache;

	std::shared_ptr<RendererSystem> renderSystem;
	std::shared_ptr<LightSystem> lightSystem;
	std::shared_ptr<PickingSystem> pickingSystem;

	Scene* scene = nullptr;

	std::shared_ptr<Camera> camera;

	void AttachUI(const std::shared_ptr<UserInterface>& ui)
	{
		userInterfaces.emplace(ui->GetName(), std::shared_ptr<UserInterface>(ui));
	}

	void DetachUI(const std::shared_ptr<UserInterface>& ui)
	{
		userInterfaces.erase(ui->GetName());
	}

	void Resize(int _width, int _height)
	{
		resourceEngine.FlushDirectQueue();

		for (int i = 0; i < swapChain->backBuffers.size(); i++)
		{
			swapChain->backBuffers[i]->GetGPUOnlyResource()->Release();
		}

		window.Resize(_width, _height);
		swapChain->swapChain->ResizeBuffers(swapChain->numBackBuffers, _width, _height, swapChain->bbFormat, NULL);

		for (int i = 0; i < swapChain->numBackBuffers; i++)
		{
			swapChain->swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChain->backBuffers[i]->GetGPUOnlyResource()));
			device->CreateRenderTargetView(swapChain->backBuffers[i]->GetGPUOnlyResource().Get(), NULL, swapChain->backBuffers[i]->GetHandle().GetCPUHandle());
		}

		swapChain->viewport.Width = _width;
		swapChain->viewport.Height = _height;
		swapChain->scissorRect.right = _width;
		swapChain->scissorRect.bottom = _height;
		
		
	}

};