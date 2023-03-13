#pragma once
#include "../ImGui/imgui.h"
#include "../ImGui/backends/imgui_impl_win32.h"
#include "../ImGui/backends/imgui_impl_dx12.h"
#include <dxgi1_4.h>
#include <tchar.h>
#include <Psapi.h>
#include "CommandList.h"
#include "DescriptorHandle.h"
#include "ImGuizmo.h"


//#include <ShObjIdl_core.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

class UserInterface
{
private:
	DescriptorHandle heapHandle;
	ImGuiIO io;
protected:

	ImVec4 clearColor;

public:
	UserInterface(const std::string& _name, ID3D12Device* _device, ID3D12DescriptorHeap* _heap, const DescriptorHandle& _descHandle, HWND _windowHandle)
	{
		name = _name;
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = ImGui::GetIO(); (void)io;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(/*_window.GetHandle()*/_windowHandle);
		heapHandle = _descHandle/*_graphics.descriptorManager.GetPassDescriptor()*/;
		ImGui_ImplDX12_Init(/*_graphics.device*/_device, 3, DXGI_FORMAT_B8G8R8A8_UNORM, /*_graphics.descriptorManager.GetResourceHeap()*/_heap, heapHandle.GetCPUHandle(), heapHandle.GetGPUHandle());
		clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	}

	virtual ~UserInterface() { printf("User Interface Destroyed\n"); }

	void BeginFrame()
	{

		if (editorMode)
		{
			ImGuizmo::Enable(true);
		}
		else
		{
			ImGuizmo::Enable(false);
		}

	}

	void Render(CommandList* _cmdList)
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), _cmdList->GetGraphicList());
	}

	virtual void Update() = 0;

	bool editorMode = false;
	std::string name = "";
};