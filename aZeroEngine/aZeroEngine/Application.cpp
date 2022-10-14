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
	double deltaTime = 0;
	std::chrono::high_resolution_clock timer;
	while (true)
	{
		auto start = timer.now();

		if (!window->Update())
			break;

		if (GetAsyncKeyState(VK_ESCAPE))
		{
			break;
		}

		if (GetAsyncKeyState('X'))
		{
			float x[4] = { 0, 1, 0, 1};
			graphics->cb->Update((void*)&x, sizeof(x));
		}
		if (GetAsyncKeyState('C'))
		{
			float x[4] = { 0, 0, 1, 1 };
			graphics->cb->Update((void*)&x, sizeof(x));
		}

		input->Update();

		graphics->camera->Update(deltaTime, *input, window->width, window->height);

		if (input->KeyDown(DIK_H))
		{
			printf("DOWN\n");
		}

		if (input->KeyUp(DIK_J))
		{
			printf("UP\n");
		}

		//static bool te = false;
		//if (GetAsyncKeyState('E') && !te)
		//{
		//	te = true;
		//	graphics->swapChain->SetFullscreen(window);
		//}
		//static bool tex = false;
		//if (GetAsyncKeyState('R') && !tex)
		//{
		//	tex = true;
		//	graphics->swapChain->SetWindowed(window, 800, 500);
		//}

		graphics->Begin();

		graphics->Update(window);

		graphics->Present();

		auto stop = timer.now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000.0f;
	}
}

Application::~Application()
{
	delete window;
	delete graphics;
}
