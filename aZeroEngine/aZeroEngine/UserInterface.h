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
	std::string name = "";

public:
	UserInterface(const std::string& _name)
		:name(_name)
	{
	}

	virtual ~UserInterface() { }
	virtual void BeginFrame() = 0;
	virtual void Update() = 0;

	const std::string GetName() const { return name; }
};