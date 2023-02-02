#include "Application.h"
#include <iostream>
#include <chrono>

Application::Application(HINSTANCE _instance, int _width, int _height)
{
	Initialize(_instance, _width, _height);
}

void Application::Initialize(HINSTANCE _instance, int _width, int _height)
{
	window = new AppWindow(WndProc, _instance, _width, _height, L"../sprites/snowflake.ico", L"../sprites/snowflakeRed.ico");
	graphics = new Graphics(window, _instance);
	input = new Input(_instance, window->windowHandle);
	sc = graphics->swapChain;
	win = window;
	init = true;
}

void Application::Run()
{
	performanceTimer.StartCountDown();
	int lastSecond = 0;
	int currentSecond = 0;
	int totalFrames = 0;

	//input->mouseDevice->Unacquire();
	//std::cout << ShowCursor(false) << "\n";

	while (true)
	{
		performanceTimer.Update();

		if (!window->Update())
			break;

		if (input->KeyDown(DIK_ESCAPE))
		{
			break;
		}

		input->Update();

		static bool editorMode = false;

		if (input->KeyDown(DIK_E))
		{
			editorMode = !editorMode;
		}

		if(!editorMode)
			graphics->camera->Update(performanceTimer.deltaTime, *input, window->width, window->height, &graphics->directCmdList, graphics->frameIndex);

		if (input->KeyDown(DIK_V))
		{
			graphics->scene->Save("C:/Users/Noah Schierenbeck/Desktop/Test", "Level1", true);
		}
		if (input->KeyDown(DIK_B) )
		{
			if (graphics->scene != nullptr)
			{
				delete graphics->scene;
			}
			graphics->scene = new Scene(graphics->ecs, graphics->vbCache, &graphics->materialManager, &graphics->resourceManager, graphics->textureCache);
			graphics->scene->Load(graphics->device, &graphics->directCmdList, graphics->frameIndex, "C:/Users/Noah Schierenbeck/Desktop/Test", "Level1");

		}

		// Rendering
		graphics->Begin();

		graphics->Render(window);

		graphics->Present();
		
	}
}

Application::~Application()
{
	delete window;
	delete graphics;
}
