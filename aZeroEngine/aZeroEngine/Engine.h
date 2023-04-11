#pragma once
#include "SwapChain.h"
#include "AppWindow.h"
#include "ECSBase.h"
#include "MaterialManager.h"
#include "VertexBufferCache.h"
#include "RenderSystem.h"
#include "LightSystem.h"
#include "PickingSystem.h"
#include "SceneManager.h"
#include "UserInterface.h"
#include "InputHandler.h"
#include "Timer.h"
#include "VertexDefinitions.h"

namespace aZero
{
	class Engine
	{
	private:

		ResourceEngine resourceEngine;
		Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

		std::unique_ptr<AppWindow> window = nullptr;
		std::unique_ptr<SwapChain> swapchain = nullptr;
		ECS ecs;
		MaterialManager materialManager;
		VertexBufferCache vbCache;
		Texture2DCache textureCache;

		RenderTarget* currentBackBuffer = nullptr;

		std::weak_ptr<Camera> mainCamera;
		std::shared_ptr<RendererSystem> renderSystem = nullptr;
		std::shared_ptr<LightSystem> lightSystem = nullptr;
		std::shared_ptr<PickingSystem> pickingSystem = nullptr;
		std::unordered_map<std::string, std::weak_ptr<UserInterface>> userInterfaces;

	public:
		Engine() = delete;

		Engine(HINSTANCE _appInstance, UINT _windowWidth, UINT _windowHeight)
			:materialManager(resourceEngine, textureCache), vbCache(resourceEngine), 
			textureCache(resourceEngine), ecs(10000)
		{
			// Modify for correct path with .exe
			window = std::make_unique<AppWindow>(WndProc, _appInstance, _windowWidth, _windowHeight, L"../sprites/snowflake.ico", L"../sprites/snowflakeRed.ico");

			HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
			if (FAILED(hr))
				throw;

#ifdef _DEBUG
			device->SetName(L"Main Device");
#endif // DEBUG

			resourceEngine.Init(device.Get());
			textureCache.Init();
			vbCache.Init();
			materialManager.Init();

			resourceEngine.Execute();

			// Uses client size since it should be the same size as the entire window
			swapchain = std::make_unique<SwapChain>(device.Get(), resourceEngine,
				window->GetHandle(), window->GetClientSize().x, window->GetClientSize().y, DXGI_FORMAT_B8G8R8A8_UNORM);

			renderSystem = ecs.RegisterSystem<RendererSystem>();
			lightSystem = ecs.RegisterSystem<LightSystem>();
			pickingSystem = ecs.RegisterSystem<PickingSystem>();

			lightSystem->Init(&resourceEngine);

			renderSystem->Init(device.Get(),
				&resourceEngine,
				&vbCache,
				lightSystem->GetLightManager(),
				&materialManager,
				swapchain.get(), _appInstance, window->GetHandle());

			pickingSystem->Init(device.Get(), &resourceEngine, &vbCache, swapchain.get());

			resourceEngine.Execute();

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO io;
			io = ImGui::GetIO(); (void)io;

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();

			ImGui_ImplWin32_Init(window->GetHandle());
			DescriptorHandle heapHandle = resourceEngine.GetDescriptorManager().GetResourceDescriptor();
			ImGui_ImplDX12_Init(device.Get(), 3, DXGI_FORMAT_B8G8R8A8_UNORM, resourceEngine.GetResourceHeap(), heapHandle.GetCPUHandle(), heapHandle.GetGPUHandle());
		}

		~Engine()
		{
			resourceEngine.Execute();
		}

		void BeginFrame()
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
			window->Update();
			resourceEngine.BeginFrame();

			currentBackBuffer = swapchain->GetBackBuffer(resourceEngine.GetFrameIndex());
			renderSystem->SetBackBuffer(currentBackBuffer);

			ID3D12DescriptorHeap* heap[] = { resourceEngine.GetResourceHeap(), resourceEngine.GetSamplerHeap() };
			resourceEngine.renderPassList.GetGraphicList()->SetDescriptorHeaps(2, heap);

			D3D12_RESOURCE_BARRIER r = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetGPUOnlyResource().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &r);
			currentBackBuffer->Clear(resourceEngine.renderPassList);

			for (auto it = userInterfaces.cbegin(); it != userInterfaces.cend();)
			{
				if (it->second.expired())
				{
					userInterfaces.erase(it++);
				}
				else
				{
					std::shared_ptr<UserInterface> ui = it->second.lock();
					ui->BeginFrame();
					it++;
				}
			}
		}

		void Render()
		{
			pickingSystem->Update();
			renderSystem->Update();
			lightSystem->Update();

			for (auto it = userInterfaces.cbegin(); it != userInterfaces.cend();)
			{
				if (it->second.expired())
				{
					userInterfaces.erase(it++);
				}
				else
				{
					std::shared_ptr<UserInterface> ui = it->second.lock();
					ui->Update();
					it++;
				}
			}
		}

		void EndFrame()
		{
			ImGui::Render();
			resourceEngine.renderPassList.GetGraphicList()->OMSetRenderTargets(1, &currentBackBuffer->GetHandle().GetCPUHandleRef(), true, nullptr);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), resourceEngine.renderPassList.GetGraphicList());

			D3D12_RESOURCE_BARRIER x = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer->GetGPUOnlyResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			resourceEngine.renderPassList.GetGraphicList()->ResourceBarrier(1, &x);

			resourceEngine.Execute();
			resourceEngine.EndFrame();

			swapchain->GetSwapChain()->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		}

		void AttachUI(std::shared_ptr<UserInterface> ui)
		{
			userInterfaces.emplace(ui->GetName(), std::shared_ptr<UserInterface>(ui));
		}

		void SetCamera(std::shared_ptr<Camera> _camera)
		{
			if (_camera)
			{
				mainCamera = _camera;
				pickingSystem->SetCamera(mainCamera);
				renderSystem->SetMainCameraGeo(mainCamera);
			}
		}

		std::shared_ptr<Scene> LoadScene(const std::string& _fileDirectory, const std::string& _filename)
		{
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(std::move(Scene(&ecs, &vbCache, &materialManager,
				&textureCache, lightSystem.get(), device.Get(), &resourceEngine)));

			if (!scene->Load(_fileDirectory, _filename))
				return nullptr;

			return scene;
		}

		std::shared_ptr<Scene> NewScene(const std::string& _name)
		{
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(std::move(Scene(&ecs, &vbCache, &materialManager,
				&textureCache, lightSystem.get(), device.Get(), &resourceEngine)));
			scene->SetName(_name);
			return scene;
		}

		void DisplayCursor(bool _display)
		{
			window->DisplayCursor(_display);
		}

		void ConfineCursor(bool _confine)
		{
			window->ConfineCursor(_confine);
		}

		int GetPickingEntityID(int _xPos, int _yPos) { return pickingSystem->GetID(_xPos, _yPos); }
		Vector2 GetClientWindowSize() const { return window->GetClientSize(); }
		uint32_t GetWindowApsectRatio() const { return window->GetAspectRatio(); }
		std::optional<Vector2> GetMouseWindowPosition() { return window->GetCursorPosition(); }

		// Getters :(
		ResourceEngine& GetResourceEngine() { return resourceEngine; }
		VertexBufferCache& GetVertexBufferCache() { return vbCache; }
		Texture2DCache& GetTexture2DCache() { return textureCache; }
		MaterialManager& GetMaterialManager() { return materialManager; }

		std::weak_ptr<LightSystem> GetLightSystem() { return lightSystem; }
		std::weak_ptr<RendererSystem> GetRenderSystem() { return renderSystem; }
		std::weak_ptr<PickingSystem> GetPickingSystem() { return pickingSystem; }

		ID3D12Device* GetDevice() { return device.Get(); }
		//
	};
}