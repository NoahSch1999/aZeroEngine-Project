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
		/*
		* Go through files and:
		*	this-> for memb funcs
		*	m_ for memb vars
		*	no c-style casts
		*	For linear alloc, resource caches, material, tree hierch, command stuff:
		*		-> document it
		* Fix crash at light update
		* Separate readback from texture
		* 
		Coding style:
			m_ for member vars
			this-> for member funcs called within member funcs
			no c-style casts

		Go through and change:
			m_ for member vars

			no _ for params

			this-> for member funcs within member funcs

			const correctness

			no c-style casts

		Implementation:

			Make so that materials are loaded per scene so scenes dont share materials

			Make paths relative so it works for both exe and vs launch

			Automate ecs reset/deletion/component-addition

		Testing:
			Check for data races etc (look at readback...)

			Check if its a problem that descriptor is reset instantly on RemoveResource? maybe defer it until the resource is released...

			Test so create and remove with descriptors works with all resources

			Check for problems with move operators/constructors and slottedmap

			Test Execute() multiple handles commandmanager

		*/

		LevelEditor editor(instance, 1920, 1080);
		editor.Run();
	}

#ifdef _DEBUG
	idxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
#endif // DEBUG

	return 0;
}