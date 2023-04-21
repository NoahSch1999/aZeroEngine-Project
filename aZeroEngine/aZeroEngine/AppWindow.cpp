#include "AppWindow.h"
#include <iostream>

bool AppWindow::Update()
{
	MSG msg = { 0 };
	bool msgReturn = 1;
	while (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

Vector2 AppWindow::GetClientSize()
{
	RECT size;
	GetClientRect(handle, &size);
	return Vector2((float)(size.right - size.left), (float)(size.bottom - size.top));
}

Vector2 AppWindow::GetWindowSize()
{
	RECT size;
	GetClientRect(handle, &size);
	return Vector2((float)(size.right - size.left), (float)(size.bottom - size.top));
}

LRESULT WndProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(_hWnd, _msg, _wParam, _lParam))
		return true;

	switch (_msg)
	{
	case WM_DESTROY:
	{
		WINDOWQUIT = true;
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYUP:
	{
		InputManager::OnKeyRelease((unsigned char)_wParam);
		break;
	}
	case WM_KEYDOWN:
	{
		InputManager::OnKeyPressed((unsigned char)_wParam);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		POINT point;
		if (GetCursorPos(&point))
		{
			if (ScreenToClient(_hWnd, &point))
			{
				InputManager::OnMouseBtnPressed(Vector2(point.x, point.y), MOUSEBTN::LEFT);
			}
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		POINT point;
		if (GetCursorPos(&point))
		{
			if (ScreenToClient(_hWnd, &point))
			{
				InputManager::OnMouseBtnReleased(Vector2(point.x, point.y), MOUSEBTN::LEFT);
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		POINT point;
		if (GetCursorPos(&point))
		{
			if (ScreenToClient(_hWnd, &point))
			{
				InputManager::OnMouseBtnPressed(Vector2(point.x, point.y), MOUSEBTN::RIGHT);
			}
		}
		break;
	}
	case WM_RBUTTONUP:
	{
		POINT point;
		if (GetCursorPos(&point))
		{
			if (ScreenToClient(_hWnd, &point))
			{
				InputManager::OnMouseBtnReleased(Vector2(point.x, point.y), MOUSEBTN::RIGHT);
			}
		}
		break;
	}
	case WM_INPUT:
	{
		UINT dataSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
		std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);

		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(_lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
		{
			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				InputManager::OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			}
		}

		break;
	}
	}

	return DefWindowProc(_hWnd, _msg, _wParam, _lParam);
}
