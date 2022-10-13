#pragma comment(lib, "dinput8.lib")
#include <iostream>
#include "Application.h"

static Application* app = nullptr;
ID3D12Debug* debug;

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{


	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
		debug->EnableDebugLayer();
	app = new Application(instance, 800, 600);
	app->Run();

	delete app;
	debug->Release();
	return 0;
}