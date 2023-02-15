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
	graphics = new Graphics(*window, _instance);
	std::vector<DescriptorHandle>bbHandles;
	bbHandles.emplace_back(graphics->rtvHeap->GetNewSlot());
	bbHandles.emplace_back(graphics->rtvHeap->GetNewSlot());
	bbHandles.emplace_back(graphics->rtvHeap->GetNewSlot());
	window->InitSwapChain(graphics->device, graphics->directCommandQueue, graphics->directCmdList, graphics->dsvHeap->GetNewSlot(), bbHandles, bbHandles.size());

	graphics->renderSystem = new BasicRendererSystem(graphics->device, graphics->directCmdList, graphics->ecs, graphics->materialManager, graphics->resourceManager, graphics->lManager, graphics->vbCache, _instance, *window);

	input = new Input(_instance, window->GetHandle());
	ui = new EditorUI(*graphics, *window);

	graphics->WaitForGPU();
}

void Application::Run()
{
	performanceTimer.StartCountDown();
	int lastSecond = 0;
	int currentSecond = 0;
	int totalFrames = 0;

	clock_t delt = 0;
	unsigned int frames = 0;
	double frameRate = 30;
	double averageFrameMilli = 33.333;

	using clocks = std::chrono::steady_clock;

	auto nextFrame = clocks::now();

	while (true)
	{
		nextFrame += std::chrono::milliseconds(1000 / 20);

		clock_t beginFrame = clock();

		performanceTimer.Update();

		if (!window->Update())
			break;

		if (input->KeyDown(DIK_B))
		{
			break;
		}


		static bool editorMode = false;
		if(!editorMode)
			input->Update();

		if (GetAsyncKeyState(VK_ESCAPE))
		{
			input->UnacquireDevices();
			if (editorMode)
				ShowCursor(false);
			else
				ShowCursor(true);
			editorMode = !editorMode;
			Sleep(100);
		}

		if (!editorMode)
		{
			Vector2 clientDimensions = window->GetClientSize();
			graphics->renderSystem->camera.Update(performanceTimer.deltaTime, *input, clientDimensions.x, clientDimensions.y, &graphics->directCmdList, graphics->frameIndex);
		}

		// Rendering
		ui->BeginFrame();

		//ui->ShowSettings();

		if (editorMode)
		{
			ImGuizmo::Enable(true);
			ui->Update();
		}
		else
		{
			ImGuizmo::Enable(false);
		}
		//ui->ShowPerformanceData();

		graphics->Begin();

		graphics->Render(window);

		ui->Render(&graphics->directCmdList);
		
		graphics->Present();

		clock_t endFrame = clock();

		delt += endFrame - beginFrame;
		frames++;

		
		if (((delt / (double)CLOCKS_PER_SEC) * 1000.0) > 1000.0)
		{
			frameRate = (double)frames * 0.5 + frameRate * 0.5;
			//std::cout << frames << "\n";
			frames = 0;
			delt -= CLOCKS_PER_SEC;
			averageFrameMilli = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);

			//std::cout << averageFrameMilli << std::endl;
		}

		//std::this_thread::sleep_until(nextFrame);
	}
}

Application::~Application()
{
	delete window;
	delete graphics;
	delete input;
	delete ui;
}
