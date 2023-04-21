#define IMGUI_DEFINE_MATH_OPERATORS
#include "pch.h"
#pragma comment(lib, "dinput8.lib")
#include <iostream>
#include "LevelEditor.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 608; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
#ifdef _DEBUG
	// cleanup so only using whats neccessary
	Microsoft::WRL::ComPtr<ID3D12Debug> spDebugController0;
	Microsoft::WRL::ComPtr<ID3D12Debug1> spDebugController1;
	D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
	spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
	spDebugController1->SetEnableGPUBasedValidation(true);

	Microsoft::WRL::ComPtr<ID3D12Debug> d3d12Debug;
	Microsoft::WRL::ComPtr<IDXGIDebug> idxgiDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug))))
		d3d12Debug->EnableDebugLayer();
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&idxgiDebug));
#endif // DEBUG

	{
		// To do:
		/*
		* Cleanup pso so multi rtv isnt hardcoded
		* Fix so a command list can be reused multiple times but still be associated with the same allocator
		* Fix so reset/close is better done + optimize, cleanup, and document the new command system
		* Check for data races etc (look at readback...)
		* Remove resource engine.
				Move its methods which manages resources to simple free functions which takes the neccessary parameters.
					Other places should synchronize resource creations etc...Engine should handle cpu sync every third frame etc tho...
		*
	    * Problem that descriptor is reset isntantly on RemoveResource? maybe defer it until the resource is released...
		* The DescriptorHandle in each resource should just be a borrowed version that is lend back to the descriptormanager in the destructor of the resource
		*	and the destruction of the resource is what is deferred using RemoveResource()
		*		Maybe even each resource can have a pointer to the resourcengine and that pointer is used in the destructor of the resource to 
		*			release the resource AND free the descriptor handle
		* Test so create and remove with descriptors work in resourcengine
		* Make paths relative so it works for both exe and vs launch
		* Sometimes crashes at scene loading... might be caused when the scene is loaded and the resources isnt setup before LightSystem::Update() is ran
			which makes the app crash at memcpy since its trying to copy to a not-setup resource...
		* Understand math for pbr (geo, fresnel)
		* Fix d-light gizmo rotation with gizmo
		* Automate ecs reset/deletion/component-addition
		*/

		LevelEditor editor(instance, 1920, 1080);
		editor.Run();
	}

#ifdef _DEBUG
	idxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
#endif // DEBUG

	return 0;
}