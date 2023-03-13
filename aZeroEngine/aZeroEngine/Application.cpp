#include "Application.h"
#include <iostream>
#include <chrono>
#include "EditorUI.h"

Application::Application(HINSTANCE _instance, int _width, int _height)
{
	Initialize(_instance, _width, _height);
}

void Application::Initialize(HINSTANCE _instance, int _width, int _height)
{
	window = new AppWindow(WndProc, _instance, _width, _height, L"../sprites/snowflake.ico", L"../sprites/snowflakeRed.ico");
	graphics = new Graphics(*window, _instance);
	std::vector<DescriptorHandle>bbHandles;
	bbHandles.emplace_back(graphics->descriptorManager.GetRTVDescriptor());
	bbHandles.emplace_back(graphics->descriptorManager.GetRTVDescriptor());
	bbHandles.emplace_back(graphics->descriptorManager.GetRTVDescriptor());
	window->InitSwapChain(graphics->device, graphics->resourceEngine, graphics->descriptorManager.GetDSVDescriptor(), bbHandles, (int)bbHandles.size());

	graphics->renderSystem = new BasicRendererSystem(graphics->device, graphics->resourceEngine, graphics->ecs, graphics->materialManager, graphics->descriptorManager, graphics->lManager, graphics->vbCache, _instance, *window);
	graphics->shadowSystem = new ShadowPassSystem(graphics->device, graphics->resourceEngine, graphics->ecs, graphics->materialManager, graphics->descriptorManager, graphics->lManager, graphics->vbCache, _instance, *window);
	
	graphics->shadowSystem->camera = &graphics->renderSystem->camera;
	graphics->renderSystem->shadowMap = &graphics->shadowSystem->shadowMapTexture;

	input = new Input(_instance, window->GetHandle());

	graphics->resourceEngine.Execute(graphics->frameIndex);

	if (graphics->scene)
	{
		for (auto& ent : graphics->scene->entities.GetObjects())
		{
			graphics->renderSystem->Bind(ent);
			graphics->shadowSystem->Bind(ent);
		}
	}
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

	std::shared_ptr<EditorUI>eui(new EditorUI("Editor", *graphics, *window));
	graphics->AttachUI(eui);

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



		eui->editorMode = editorMode;
		
		graphics->BeginFrame();

		if (!editorMode)
		{
			Vector2 clientDimensions = window->GetClientSize();
			graphics->renderSystem->camera.Update(graphics->resourceEngine, (float)performanceTimer.deltaTime, *input, (UINT)clientDimensions.x, (UINT)clientDimensions.y, graphics->frameIndex);
		}

		graphics->lManager.Update(graphics->frameIndex);

		//eui->ShowMaterialPreview();
		graphics->Render(window);

		

		

		graphics->EndFrame();

		clock_t endFrame = clock();

		delt += endFrame - beginFrame;
		frames++;

		
		if (((delt / (double)CLOCKS_PER_SEC) * 1000.0) > 1000.0)
		{
			frameRate = (double)frames * 0.5 + frameRate * 0.5;
			frames = 0;
			delt -= CLOCKS_PER_SEC;
			averageFrameMilli = 1000.0 / (frameRate == 0 ? 0.001 : frameRate);
		}
	}

	graphics->resourceEngine.Execute(0);
}

Application::~Application()
{
	delete window;
	delete graphics;
	delete input;
}
