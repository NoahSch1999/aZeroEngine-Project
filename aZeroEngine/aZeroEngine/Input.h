#pragma once
#include <dinput.h>
#include <iostream>

class Input
{
private:
	BYTE diKeys[256];
	bool pressed[256];
	bool pressedLastFrame[256];

	LPDIRECTINPUT8 dInput;

	IDirectInputDevice8* keyboardDevice;
	LPDIRECTINPUTDEVICE8 lpKeyboardDevice;

	IDirectInputDevice8* mouseDevice;
	LPDIRECTINPUTDEVICE8 lpMouseDevice;

public:
	Input(HINSTANCE _instance, HWND _handle)
	{
		memset(&pressedLastFrame, false, sizeof(pressedLastFrame));
		HRESULT hr;

		hr = DirectInput8Create(_instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dInput, NULL);
		if (FAILED(hr))
			throw;

		// Mouse
		hr = dInput->CreateDevice(GUID_SysMouse, &mouseDevice, NULL);
		if (FAILED(hr))
			throw;

		hr = mouseDevice->SetCooperativeLevel(_handle, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
		if (FAILED(hr))
			throw;

		hr = mouseDevice->SetDataFormat(&c_dfDIMouse);
		if (FAILED(hr))
			throw;

		// Keyboard
		hr = dInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL);
		if (FAILED(hr))
			throw;

		hr = keyboardDevice->SetCooperativeLevel(_handle, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
		if (FAILED(hr))
			throw;

		hr = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hr))
			throw;
	}

	void Update()
	{
		mouseDevice->Acquire();
		keyboardDevice->Acquire();

		keyboardDevice->GetDeviceState(sizeof(diKeys), (LPVOID)&diKeys);

		mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&currentState);

		if (currentState.lX != lastState.lX || currentState.lY != lastState.lY)
		{
			mouseMoved = true;
			lastState = currentState;
		}
		else
		{
			mouseMoved = false;
		}

		if (lastWheelValue != currentState.lZ)
		{
			mouseWheelMoved = true;
			lastWheelValue = currentState.lZ;
		}
		else
		{
			lastWheelValue = false;
		}
	}

	bool KeyHeld(const char key)
	{
		if (diKeys[key] & 0x80)
		{
			return true;
		}
		return false;
	}

	// Make better looking... Quickfix...
	bool KeyUp(const char key)
	{
		if (diKeys[key] & 0x80)
		{
			pressed[key] = true;
		}
		else
		{
			pressed[key] = false;
		}

		if (!pressed[key] && pressedLastFrame[key])
		{
			pressedLastFrame[key] = pressed[key];
			return true;
		}

		pressedLastFrame[key] = pressed[key];
		return false;
	}

	bool KeyDown(const char key)
	{
		// Doesn't work for KeyUp since this asumes pressedLastFrame[key] is false the first frame
		if ((diKeys[key] & 0x80) && !pressedLastFrame[key]) 
		{
			pressedLastFrame[key] = true;
			return true;
		}
		else if(!(diKeys[key] & 0x80))
		{
			pressedLastFrame[key] = false;
		}
		return false;
	}

	bool mouseMoved = false;
	bool mouseWheelMoved = false;
	bool mouseWheelHeld = false;
	DIMOUSESTATE currentState;
	DIMOUSESTATE lastState;
	LONG lastWheelValue;
};

