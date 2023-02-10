#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_win32.h"
#include "../ImGui/backends/imgui_impl_dx12.h"
#include <dxgi1_4.h>
#include <tchar.h>
#include "Scene.h"
#include <Psapi.h>
#include <filesystem>
#include "ImGuizmo.h"
//#include <ShObjIdl_core.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#include "Graphics.h"
#include "AppWindow.h"


using Task = std::function<void()>;

class UserInterface
{
private:
	DescriptorHandle heapHandle;
	ImGuiIO io;
protected:
	Graphics& graphics;
	AppWindow& window;
	ImVec4 clearColor;

public:
	UserInterface(Graphics& _graphics, AppWindow& _window)
		:graphics(_graphics), window(_window)
	{

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(_window.windowHandle);
		heapHandle = _graphics.resourceManager.GetPassDescriptor();
		ImGui_ImplDX12_Init(_graphics.device, 3, DXGI_FORMAT_B8G8R8A8_UNORM, _graphics.resourceManager.GetResourceHeap(), heapHandle.GetCPUHandle(), heapHandle.GetGPUHandle());

		clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	}

	void BeginFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void Render(CommandList* _cmdList)
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _cmdList->GetGraphicList());
	}

};

class EditorUI : public UserInterface
{
private:

public:

	

	std::string selectedEntityStr = "";
	std::string lastSelectedEntityStr = "";
	int selectedEntityID = -1;
	int lastSelectedEntityID = -1;

	std::string selectedMaterialStr = "";
	int selectedMaterialID = -1;

	EditorUI(Graphics& _graphics, AppWindow& _window)
		:UserInterface(_graphics, _window)
	{

	}

	void Update();
};