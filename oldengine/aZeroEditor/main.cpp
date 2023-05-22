#include <iostream>
#include "../aZeroEngine/Engine.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 608; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

#ifdef _DEBUG
ID3D12Debug* d3d12Debug;
IDXGIDebug* idxgiDebug;
#endif // DEBUG

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
#ifdef _DEBUG

	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
	spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
	spDebugController1->SetEnableGPUBasedValidation(true);

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug))))
		d3d12Debug->EnableDebugLayer();
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&idxgiDebug));
#endif // DEBUG
    std::cout << "Hello World!\n";

	{
		aZero::Engine engine(instance, 1920, 1080);
		std::shared_ptr<Scene> scene = engine.LoadScene("..\\scenes\\", "new");
		std::shared_ptr<Camera> camera = engine.NewCamera("Main Camera", 0.006f);
		engine.SetCamera(camera);

		Timer timer;
		timer.StartCountDown();

		while (true)
		{
			timer.Update();

			engine.BeginFrame();

			static bool editorMode = false;
			if (InputManager::KeyDown(VK_ESCAPE))
			{
				editorMode = !editorMode;
				engine.DisplayCursor(editorMode);
				engine.ConfineCursor(!editorMode);
				camera->ToggleActive(!editorMode);
			}

			if (InputManager::KeyDown('B'))
			{
				break;
			}

			if (camera->Active())
			{
				Vector2 clientDimensions = engine.GetClientWindowSize();
				camera->Update(*engine.GetResourceEngine(), timer.deltaTime, (float)clientDimensions.x / (float)clientDimensions.y, engine.GetFrameIndex());
			}

			if (InputManager::MouseBtnDown(LEFT))
			{
				Vector2 cursorPosition = InputManager::GetClickedPosition();
				int pickValue = engine.GetPickingEntityID(cursorPosition.x, cursorPosition.y);
				if (pickValue != -1)
				{
					Transform* comp = scene->GetComponentForEntity<Transform>(scene->GetEntity(pickValue));
					if (comp)
					{
						comp->SetTranslation(comp->GetTranslation() + Vector3(0, 1, 0));
						comp->Update(*engine.GetResourceEngine(), engine.GetFrameIndex());
					}
				}
			}

			engine.Render();

			engine.EndFrame();
			InputManager::EndFrame();
		}
	}

#ifdef _DEBUG
	HRESULT  hr = idxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
	d3d12Debug->Release();
	idxgiDebug->Release();
#endif // DEBUG
}