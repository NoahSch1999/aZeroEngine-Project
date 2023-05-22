#pragma once
#include "SwapChain.h"
#include "AppWindow.h"
#include "MaterialManager.h"
#include "ModelCache.h"
#include "RenderSystem.h"
#include "LightSystem.h"
#include "ParentSystem.h"
#include "PhysicSystem.h"
#include "Scene.h"
#include "UserInterface.h"
#include "InputHandler.h"
#include "Timer.h"
#include "VertexDefinitions.h"

#include "ECS.h"

namespace aZero
{
	class Engine
	{
	private:
		UINT m_frameIndex = 0;
		UINT64 m_frameCount = 0;
		DescriptorManager m_descriptorManager;
		ResourceTrashcan m_resourceBin;
		std::unique_ptr<CommandManager> m_commandManager;

		Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

		std::unique_ptr<AppWindow> window = nullptr;
		std::unique_ptr<SwapChain> swapchain = nullptr;

		MaterialManager materialManager;
		ModelCache modelCache;
		Texture2DCache textureCache;

		Texture* currentBackBuffer = nullptr;

		// New ECS
		aZeroECS::EntityManager entityManager;
		aZeroECS::ComponentManager componentManager;
		aZeroECS::SystemManager systemManager;

		//

		std::weak_ptr<Camera> mainCamera;
		std::shared_ptr<RendererSystem> renderSystem = nullptr;
		std::shared_ptr<LightSystem> lightSystem = nullptr;
		std::shared_ptr<ParentSystem> parentSystem = nullptr;
		std::shared_ptr<PhysicSystem> m_physicSystem = nullptr;

		std::unordered_map<std::string, std::weak_ptr<UserInterface>> userInterfaces;

	private:
		void Flush()
		{
			if (m_frameIndex % 3 == 0)
			{
				m_commandManager->flushCPU();

				if (m_resourceBin.resources.size() > 0)
				{
					m_resourceBin.resources.clear();
				}
			}
		}

	public:

		aZeroECS::EntityManager& GetEntityManager() { return entityManager; }
		aZeroECS::ComponentManager& GetComponentManager() { return componentManager; }
		aZeroECS::SystemManager& GetSystemManager() { return systemManager; }

		Engine() = delete;

		Engine(HINSTANCE _appInstance, UINT _windowWidth, UINT _windowHeight)
			:materialManager(textureCache), modelCache(m_resourceBin),
			textureCache(m_descriptorManager, m_resourceBin), componentManager(systemManager), entityManager(componentManager, systemManager)
		{
			componentManager.RegisterComponent<Transform>();
			componentManager.RegisterComponent<Mesh>();
			componentManager.RegisterComponent<MaterialComponent>();
			componentManager.RegisterComponent<PointLightComponent>();
			componentManager.RegisterComponent<DirectionalLightComponent>();
			componentManager.RegisterComponent<RigidBody>();

			HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device));
			if (FAILED(hr))
				throw;

			m_descriptorManager.Init(device.Get(), 10000, 20, 100, 100);
			m_commandManager = std::make_unique<CommandManager>(device.Get(), 15, 15, 15);

			// Modify for correct path with .exe
			window = std::make_unique<AppWindow>(WndProc, _appInstance, _windowWidth, _windowHeight, L"../sprites/snowflake.ico", L"../sprites/snowflakeRed.ico");

#ifdef _DEBUG
			device->SetName(L"Main Device");
#endif // DEBUG

			{
				GraphicsContextHandle context = m_commandManager->getGraphicsContext();
				textureCache.Init(device.Get(), context, 0);
				modelCache.Init(device.Get(), context, 0);
				materialManager.Init(device.Get(), context, 0);
				m_commandManager->executeContext(context);
			}

			// Uses client size since it should be the same size as the entire window
			swapchain = std::make_unique<SwapChain>(device.Get(), m_commandManager->getGraphicsQueue(), m_descriptorManager, m_resourceBin,
				window->GetHandle(), window->GetClientSize().x, window->GetClientSize().y, DXGI_FORMAT_B8G8R8A8_UNORM);

			lightSystem = systemManager.RegisterSystem<LightSystem>(componentManager, device.Get(), m_resourceBin);
			
			renderSystem = systemManager.RegisterSystem<RendererSystem>(componentManager, device.Get(), *m_commandManager.get(),
				m_resourceBin, m_descriptorManager,
				&modelCache,
				lightSystem->GetLightManager(),
				&materialManager,
				swapchain.get(), _appInstance, window->GetHandle());

			parentSystem = systemManager.RegisterSystem<ParentSystem>(componentManager);

			m_physicSystem = systemManager.RegisterSystem<PhysicSystem>(componentManager);

			renderSystem->parentSystem = parentSystem.get();

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.IniFilename = NULL;

			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Crashes the app... why?
			//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
			//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

			ImGui::StyleColorsDark();

			ImGui_ImplWin32_Init(window->GetHandle());
			DescriptorHandle heapHandle = m_descriptorManager.getResourceDescriptor();
			ImGui_ImplDX12_Init(device.Get(), 3, DXGI_FORMAT_B8G8R8A8_UNORM, m_descriptorManager.getResourceHeap(), heapHandle.getCPUHandle(), heapHandle.getGPUHandle());
			ImGui::LoadIniSettingsFromDisk("uiSettings.abzui");
		}

		~Engine()
		{
			ImGui::SaveIniSettingsToDisk("uiSettings.abzui");
			this->Flush();
		}

		void BeginFrame()
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
			window->Update();

			m_frameIndex = m_frameCount % 3;
			aZeroECS::ComponentArray<MaterialComponent>& arr = componentManager.GetComponentArray<MaterialComponent>();

			currentBackBuffer = swapchain->getBackBuffer(m_frameIndex);
			renderSystem->BeginFrame(currentBackBuffer, m_frameIndex);
			lightSystem->BeginFrame(m_frameIndex);

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

			parentSystem->Update();
			m_physicSystem->Update();

			{
				CopyContextHandle context = m_commandManager->getCopyContext();
				lightSystem->Update(context.getList());
				m_commandManager->executeContext(context);
			}
			
			m_commandManager->graphicsWaitFor(m_commandManager->getCopyQueue(), m_commandManager->getCopyQueue().getLastSignalValue());
			renderSystem->Update();
		}

		void EndFrame()
		{
			ImGui::Render();
			
			{
				GraphicsContextHandle context = m_commandManager->getGraphicsContext();

				context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
				ID3D12DescriptorHeap* heap[] = { m_descriptorManager.getResourceHeap(), m_descriptorManager.getSamplerHeap() };
				context.setDescriptorHeaps(2, heap);
				context.setOMRenderTargets(1, &currentBackBuffer->getRTVDSVHandle().getCPUHandleRef(), true, nullptr);

				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.getList());

				context.transitionTexture(*currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT);
				m_commandManager->executeContext(context);
			}

			this->Flush();
			m_frameCount++;

			swapchain->getSwapChain()->Present(0, DXGI_PRESENT_ALLOW_TEARING);

			/*GraphicsContextHandle context = m_commandManager->getGraphicsContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(nullptr, (void*)context.getList());
			m_commandManager->executeContext(context);*/
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
				renderSystem->SetMainCameraGeo(mainCamera);
			}
		}

		std::shared_ptr<Scene> LoadScene(GraphicsContextHandle& context, const std::string& _fileDirectory, const std::string& _filename)
		{
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(std::move(Scene(&modelCache, &materialManager,
				lightSystem.get(), &componentManager, &entityManager)));

			if (!scene->Load(device.Get(), context, m_frameIndex, _fileDirectory, _filename))
				return nullptr;

			return scene;
		}

		std::shared_ptr<Scene> NewScene(const std::string& _name)
		{
			std::shared_ptr<Scene> scene = std::make_shared<Scene>(std::move(Scene(&modelCache, &materialManager,
				lightSystem.get(), &componentManager, &entityManager)));
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

		int GetPickingEntityID(int _xPos, int _yPos) { return renderSystem->GetPickingID(_xPos, _yPos); }
		DXM::Vector2 GetClientWindowSize() const { return window->GetClientSize(); }
		uint32_t GetWindowApsectRatio() const { return window->GetAspectRatio(); }
		std::optional<DXM::Vector2> GetMouseWindowPosition() { return window->GetCursorPosition(); }

		// Getters :(
		UINT GetFrameIndex() const { return m_frameIndex; }
		ResourceTrashcan& GetResourceTrashcan() { return m_resourceBin; }
		CommandManager& GetCommandManager() { return *m_commandManager.get(); }
		ModelCache& GetModelCache() { return modelCache; }
		Texture2DCache& GetTexture2DCache() { return textureCache; }
		MaterialManager& GetMaterialManager() { return materialManager; }
		DescriptorManager& GetDescriptorManager() { return m_descriptorManager; }
		std::weak_ptr<LightSystem> GetLightSystem() { return lightSystem; }
		std::weak_ptr<RendererSystem> GetRenderSystem() { return renderSystem; }
		std::weak_ptr<ParentSystem> GetParentSystem() { return parentSystem; }
		std::weak_ptr<PhysicSystem> GetPhysicSystem() { return m_physicSystem; }

		ID3D12Device* GetDevice() { return device.Get(); }
		//
	};
}