#pragma once
#include <Windows.h>
#include <string>
#include "imgui.h"
#include "SwapChain.h"
#include "InputHandler.h"
#include <optional>
#include <iostream>

inline bool WINDOWQUIT;

// https://learn.microsoft.com/en-us/windows/win32/gdi/positioning-objects-on-multiple-display-monitors
// https://walbourn.github.io/care-and-feeding-of-modern-swap-chains-3/
class AppWindow
{
private:
	HCURSOR cursor;
	HWND handle;
	HINSTANCE instance;
	UINT message;
	WNDCLASS wc = { 0 };
	bool cursorShown = true;
	bool cursorConfined = true;
	
public:

	AppWindow() = default;

	template <typename T>
	AppWindow(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);

	~AppWindow() = default;

	template<typename T>
	void Init(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);

	bool Update();

	std::optional<Vector2> GetCursorPosition() {
		POINT point;
		if (GetCursorPos(&point))
		{
			if (ScreenToClient(handle, &point))
			{
				return Vector2(point.x, point.y);
			}
		}
		return {};
	}

	Vector2 GetClientSize();

	Vector2 GetWindowSize();

	uint32_t GetAspectRatio() { return GetClientSize().x / GetClientSize().y; }

	HWND& GetHandle() { return handle; }
	HINSTANCE& GetInstance() { return instance; }

	void Resize(int _width, int _height)
	{
		SetWindowPos(handle, handle, 0, 0, _width, _height, SWP_SHOWWINDOW | SWP_NOZORDER);
	}

	void DisplayCursor(bool _show)
	{
		if (cursorShown == _show)
			return;

		ShowCursor(_show);
		cursorShown = _show;
	}

	void ConfineCursor(bool _confine)
	{
		if (cursorConfined == _confine)
			return;

		if (_confine)
		{
			RECT rect;
			GetClientRect(handle, &rect);

			POINT ul;
			ul.x = rect.left;
			ul.y = rect.top;

			POINT lr;
			lr.x = rect.right;
			lr.y = rect.bottom;

			MapWindowPoints(handle, nullptr, &ul, 1);
			MapWindowPoints(handle, nullptr, &lr, 1);

			rect.left = ul.x;
			rect.top = ul.y;

			rect.right = lr.x;
			rect.bottom = lr.y;

			ClipCursor(&rect);
		}
		else
		{
			ClipCursor(nullptr);
		}

		cursorConfined = _confine;
	}
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

template<typename T>
inline AppWindow::AppWindow(T* _wndProc, HINSTANCE _instance, int _width, int _height,
	const std::wstring& _smallIconPath, const std::wstring& _bigIconPath)
{
	Init(_wndProc, _instance, _width, _height, _smallIconPath, _bigIconPath);
}

template<typename T>
inline void AppWindow::Init(
	T* _wndProc, HINSTANCE _instance, int _width, int _height,
	const std::wstring& _smallIconPath, const std::wstring& _bigIconPath)
{
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
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	cursor = LoadCursor(_instance, IDC_ARROW);
	wc.hCursor = cursor;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;


	// Register the window to the OS
	if (!RegisterClass(&wc))
		throw;

	handle = CreateWindowExW(0, className, (LPCWSTR)L"aZero Engine", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, _width, _height, 0, 0, _instance, 0);
	if (handle == 0)
		throw;

#ifdef _DEBUG
	AllocConsole();
	(void)freopen("conout$", "w", stdout);
#endif

	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);
	DisplayCursor(false);

	instance = _instance;

	SendMessage(handle, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
	SendMessage(handle, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);

	RAWINPUTDEVICE rid;

	rid.usUsagePage = 0x0001;          // HID_USAGE_PAGE_GENERIC
	rid.usUsage = 0x02;              // HID_USAGE_GENERIC_GAMEPAD
	rid.dwFlags = 0;                 // adds game pad
	rid.hwndTarget = handle;

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE)
	{
		throw;
	}

	WINDOWQUIT = false;
};
