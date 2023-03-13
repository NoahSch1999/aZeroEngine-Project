#pragma once
#include <Windows.h>
#include <string>
#include "imgui.h"
#include "SwapChain.h"

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
	SwapChain swapChain;
	DXGI_FORMAT dsvFormat;
	DXGI_FORMAT bbFormat;
public:

	AppWindow() = default;

	template <typename T>
	AppWindow(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);

	~AppWindow() = default;

	template<typename T>
	void Init(T* _wndProc, HINSTANCE _instance, int _width, int _height,
		const std::wstring& _smallIconPath, const std::wstring& _bigIconPath);

	void InitSwapChain(ID3D12Device* _device, ResourceEngine& _resourceEngine,
		DescriptorHandle _dsvHandle,
		std::vector<DescriptorHandle> _bbHandles,
		int _numBackBuffers);

	bool Update();

	HCURSOR& GetCursor() { return cursor; }

	Vector2 GetClientSize();

	Vector2 GetWindowSize();

	HWND& GetHandle() { return handle; }
	HINSTANCE& GetInstance() { return instance; }

	DXGI_FORMAT GetDSVFormat() { return dsvFormat; }
	DXGI_FORMAT GetBBFormat() { return bbFormat; }
	SwapChain& GetSwapChain() { return swapChain; }
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static inline LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	static bool minimized = false;
	static bool paused = false;

	if (ImGui_ImplWin32_WndProcHandler(_hWnd, _msg, _wParam, _lParam))
		return true;

	switch (_msg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:
		{

			if (_wParam == SIZE_MINIMIZED)
			{
				if (!minimized)
				{
					minimized = true;
					printf("MINIMIZED\n");
				}
			}
			else if (minimized)
			{
				minimized = false;
				printf("MAXIMIZED\n");
			}

			break;
		}
	}

	return DefWindowProc(_hWnd, _msg, _wParam, _lParam);
}

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

	instance = _instance;

	SendMessage(handle, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
	SendMessage(handle, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
}
