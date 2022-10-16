#pragma once
#include "AppWindow.h"
#include "Graphics.h"
#include "Timer.h"

class Application
{
private:
	void Begin();
	void Present();

public:
	AppWindow* window;
	Graphics* graphics;
	Input* input;
	Timer performanceTimer;

	Application(HINSTANCE _instance, int _width, int _height);
	~Application();
	void Initialize(HINSTANCE _instance, int _width, int _height);
	void Run();
};

