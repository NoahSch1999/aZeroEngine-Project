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

	while (true)
	{
		performanceTimer.Update();

		if (!window->Update())
			break;

		if (input->KeyDown(DIK_ESCAPE))
		{
			break;
		}

		if (input->KeyDown(DIK_F))
		{
			HRESULT hr = graphics->swapChain->swapChain->SetFullscreenState(true, NULL);
			if (FAILED(hr))
				throw;
			graphics->frameCount = 0;
		}

		if (input->KeyDown(DIK_G))
		{
			HRESULT hr = graphics->swapChain->swapChain->SetFullscreenState(false, NULL);
			if (FAILED(hr))
				throw;
			graphics->frameCount = 0;
		}

		input->Update();

		graphics->camera->Update(performanceTimer.deltaTime, *input, window->width, window->height);

		// Rendering
		graphics->Begin();

		graphics->Update(window);

		graphics->Present();
		
		// FPS
		currentSecond = performanceTimer.StopCountDown<std::chrono::seconds>();
		if (currentSecond != lastSecond)
		{
			std::cout << "FPS: " << totalFrames << std::endl;
			totalFrames = 0;
		}

		lastSecond = performanceTimer.StopCountDown<std::chrono::seconds>();
		++totalFrames;
		//
	}
}

Application::~Application()
{
	delete window;
	delete graphics;
}
