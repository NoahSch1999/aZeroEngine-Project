#include "AppWindow.h"

AppWindow::~AppWindow()
{
}

bool AppWindow::Update()
{
	MSG msg = { 0 };
	bool msgReturn = 1;
	while (PeekMessage(&msg, windowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//if (msg.message == WM_QUIT)
	//{
	//	return false;
	//}

	return true;
}
