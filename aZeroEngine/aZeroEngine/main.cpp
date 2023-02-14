#include "pch.h"

#pragma comment(lib, "dinput8.lib")
#include <iostream>
#include "Application.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 608; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

//static Application* app = nullptr;
#ifdef _DEBUG
ID3D12Debug* d3d12Debug;
IDXGIDebug* idxgiDebug;
#endif // DEBUG

class XXX
{
public:
	XXX() = default;
	~XXX()
	{
		printf("s");
	}
};

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
#ifdef _DEBUG
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug))))
		d3d12Debug->EnableDebugLayer();
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&idxgiDebug));
#endif // DEBUG

	{
		Application app(instance, 1920, 1080/*800, 600*/);
		//app = new Application

		app.Run();
	}
	//delete app;
#ifdef _DEBUG
	HRESULT  hr = idxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	d3d12Debug->Release();
	idxgiDebug->Release();
#endif // DEBUG

	return 0;
}