#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>
#include "LevelEditor.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 610; }

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
		*	
		* Make texture and mesh drag/drop area scrollable
		* 
		* Fix so transform is kept when adding rb
		* Fix so transform is kept when parenting/childing
		* 
		* 
		Coding style:
			m_ for member vars
			this-> for methods called within methods
			no c-style casts
			functions and methods should be camelCase

		Testing:

			Check if its a problem that descriptor is reset instantly on RemoveResource? maybe defer it until the resource is released...
				Some other resource might take the descriptor and use it. Is that a problem?

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