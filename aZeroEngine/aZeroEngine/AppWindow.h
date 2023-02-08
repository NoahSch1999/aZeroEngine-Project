#pragma once
#include <Windows.h>
#include <string>
#include "SwapChain.h"
#include "imgui.h"

class AppWindow
{
private:
	HCURSOR cursor;
public:
	HWND windowHandle;
	HINSTANCE instance;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	UINT width;
	UINT height;
	WNDCLASS wc = { 0 };

	AppWindow() = default;
	template <typename T>
	AppWindow(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);
	~AppWindow();
	template<typename T>
	void Initialize(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);
	bool Update();

	HCURSOR& GetCursor() { return cursor; }
};

static SwapChain* sc = nullptr;
static AppWindow* win = nullptr;
static bool init = false;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static inline LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(_hWnd, _msg, _wParam, _lParam))
		return true;

	switch (_msg)
	{
		case WM_KEYDOWN:
		{
			//if (_wParam == VK_ESCAPE)
			//{
			//	DestroyWindow(_hWnd);
			//	return 0;
			//}
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{
			if(init)
				sc->OnResize(win->windowHandle);
			break;
		}
	}

	return DefWindowProc(_hWnd, _msg, _wParam, _lParam);
}

template<typename T>
inline AppWindow::AppWindow(T* _wndProc, HINSTANCE _instance, int _width, int _height,
	const std::wstring& _smallIconPath, const std::wstring& _bigIconPath)
{
	Initialize(_wndProc, _instance, _width, _height, _smallIconPath, _bigIconPath);
}

template<typename T>
inline void AppWindow::Initialize(T* _wndProc, HINSTANCE _instance, int _width, int _height, 
	const std::wstring& _smallIconPath, const std::wstring& _bigIconPath)
{
	//std::wstring path = L"C:/Projects/aZeroEngine/aZeroEngine/sprites/snowflake.ico";

	HICON smallIcon = reinterpret_cast<HICON>(LoadImage(nullptr, _smallIconPath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
	HICON bigIcon = reinterpret_cast<HICON>(LoadImage(nullptr, _bigIconPath.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE));

	LPCWSTR className = L"MyWindow";
	wc = { 0 };
	wc.lpfnWndProc = _wndProc;
	wc.lpszClassName = className;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _instance;
	//wc.hIcon = LoadIcon(_instance, MAKEINTRESOURCE(path.c_str()));
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cursor = LoadCursor(_instance, IDC_ARROW);
	wc.hCursor = cursor;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;

	// Register the window to the OS
	if (!RegisterClass(&wc))
		throw;

	windowHandle = CreateWindowEx(0, className, (LPCWSTR)L"aZero Engine", WS_OVERLAPPEDWINDOW, 0, 0, _width, _height, 0, 0, _instance, 0);
	if (windowHandle == 0)
		throw;

#ifdef _DEBUG
	AllocConsole();
	(void)freopen("conout$", "w", stdout);
#endif

	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);

	width = _width;
	height = _height;
	instance = instance;

	//HICON icon = LoadIcon(_instance, MAKEINTRESOURCE(path.c_str()));
	SendMessage(windowHandle, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
	SendMessage(windowHandle, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
}
