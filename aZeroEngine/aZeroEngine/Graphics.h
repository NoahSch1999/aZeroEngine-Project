#pragma once
#include <iostream>
#include "SwapChain.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "UserInterface.h"
#include "ResourceEngine.h"

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

	RenderTarget* currentBackBuffer;
	int nextSyncSignal = 0;
	int frameIndex = 0;
	int frameCount = 0;

	ECS ecs;
	ResourceEngine resourceEngine;

	DescriptorManager descriptorManager;
	MaterialManager materialManager;
	LightManager lManager;

	VertexBufferCache vbCache;
	Texture2DCache textureCache;

	BasicRendererSystem* renderSystem;
	ShadowPassSystem* shadowSystem;

	Scene* scene = nullptr;

	void AttachUI(const std::shared_ptr<UserInterface>& ui)
	{
		userInterfaces.emplace(ui->name, std::shared_ptr<UserInterface>(ui));
	}

	void DetachUI(const std::shared_ptr<UserInterface>& ui)
	{
		userInterfaces.erase(ui->name);
	}
};