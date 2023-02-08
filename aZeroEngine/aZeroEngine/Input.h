#pragma once
#include <dinput.h>
#include <iostream>
#include <GameInput.h>

namespace MouseButtons
{
	enum BUTTON { LEFT, RIGHT, MIDDLE, SIDE };
}

enum InpButtons { NONE, LEFT, RIGHT, MIDDLE, BTN4, BTN5 };

class GameInput
{
private:
	IGameInput* gameInput = nullptr;
	IGameInputDevice* keyboardDevice = nullptr;

	IGameInputDevice* mouseDevice = nullptr;
	GameInputMouseState mouseState;
	bool lastDownMouse[5];
	bool downMouse[5];
	int lastMouseWheelDir = 0;
	int mouseWheelDir = 0;

	int mouseXDir = 0;
	int lastMouseXDir = 0;
	int mouseYDir = 0;
	int lastMouseYDir = 0;
	int rawMouseX = 0;
	int rawMouseY = 0;
	float vSensitivity = 1.f;
	float hSensitivity = 1.f;

public:

	int GetMouseWheelDirection() { return mouseWheelDir; }

	Vector2 GetMouseDirections() { return Vector2(mouseXDir, mouseYDir); }
	Vector2 GetRawMouseDirections() { return Vector2(rawMouseX, rawMouseY); }

	void SetVerticalSensitivity(float _newSens) { vSensitivity = _newSens; }
	float GetVerticalSensitivity() { return vSensitivity; }

	void SetHorizontalSensitivity(float _newSens) { hSensitivity = _newSens; }
	float GetHorizontalSensitivity() { return hSensitivity; }

	GameInput()
	{
		HRESULT hr = GameInputCreate(&gameInput);
		if (FAILED(hr))
			throw;
	}

	~GameInput()
	{
		if (gameInput != nullptr)
			gameInput->Release();

		if (keyboardDevice != nullptr)
			keyboardDevice->Release();

		if (mouseDevice != nullptr)
			mouseDevice->Release();
	}

	bool IsMouseButtonDown(InpButtons _btn)
	{
		if (downMouse[_btn])
		{
			if (!lastDownMouse[_btn])
				return true;
		}

		return false;
	}

	void PrepNextFrame()
	{
		lastMouseWheelDir = mouseState.wheelY;
		memcpy(lastDownMouse, downMouse, sizeof(downMouse));
	}

	void PollInputs()
	{
		IGameInputReading* reading;

		if (SUCCEEDED(gameInput->GetCurrentReading(GameInputKindMouse, mouseDevice, &reading)))
		{
			if (!mouseDevice)
				reading->GetDevice(&mouseDevice);

			
			reading->GetMouseState(&mouseState);
			reading->Release();

			if (mouseState.wheelY > lastMouseWheelDir)
			{
				mouseWheelDir = 1;
			}
			else if (mouseState.wheelY < lastMouseWheelDir)
			{
				mouseWheelDir = -1;
			}
			else
			{
				mouseWheelDir = 0;
			}

			if (mouseState.positionX > lastMouseXDir)
			{
				mouseXDir = 1;
				rawMouseX = mouseState.positionX - lastMouseXDir;
			}
			else if (mouseState.positionX < lastMouseXDir)
			{
				mouseXDir = -1;
				rawMouseX = mouseState.positionX - lastMouseXDir;
			}
			else
			{
				mouseXDir = 0;
				rawMouseX = 0;
			}

			if (mouseState.positionY > lastMouseYDir)
			{
				mouseYDir = 1;
				rawMouseY = mouseState.positionY - lastMouseYDir;
			}
			else if (mouseState.positionY < lastMouseYDir)
			{
				mouseYDir = -1;
				rawMouseY = mouseState.positionY - lastMouseYDir;
			}
			else
			{
				mouseYDir = 0;
				rawMouseY = 0;
			}

			/*if (rawMouseY != 0)
			{
				std::cout << rawMouseY << std::endl;
			}*/

			lastMouseXDir = mouseState.positionX;
			lastMouseYDir = mouseState.positionY;

			if (mouseState.buttons == GameInputMouseNone)
				downMouse[0] = true;
			else
				downMouse[0] = false;

			if (mouseState.buttons == GameInputMouseLeftButton)
				downMouse[1] = true;
			else
				downMouse[1] = false;

			if (mouseState.buttons == GameInputMouseRightButton)
				downMouse[2] = true;
			else
				downMouse[2] = false;

			if (mouseState.buttons == GameInputMouseMiddleButton)
				downMouse[3] = true;
			else
				downMouse[3] = false;

			if (mouseState.buttons == GameInputMouseButton4)
				downMouse[4] = true;
			else
				downMouse[4] = false;

			if (mouseState.buttons == GameInputMouseButton5)
				downMouse[5] = true;
			else
				downMouse[5] = false;
		}

		if (SUCCEEDED(gameInput->GetCurrentReading(GameInputKindKeyboard, keyboardDevice, &reading)))
		{
			if (!keyboardDevice)
				reading->GetDevice(&keyboardDevice);

			/*uint32_t keyCount;
			GameInputKeyState state;
			reading->GetKeyState(keyCount, &state);
			reading->Release();*/

			// use state...

			//
		}
		else
		{
			keyboardDevice->Release();
			keyboardDevice = nullptr;
		}
	}
};

class Input
{
private:
	BYTE diKeys[256];
	bool pressed[256];
	bool pressedLastFrame[256];

	LPDIRECTINPUT8 dInput;

	IDirectInputDevice8* keyboardDevice;
	LPDIRECTINPUTDEVICE8 lpKeyboardDevice;

	LPDIRECTINPUTDEVICE8 lpMouseDevice;
	bool mPressed[4];
	bool mPressedLastFrame[4];

public:
	IDirectInputDevice8* mouseDevice;

	Input(HINSTANCE _instance, const HWND& _handle)
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

		ShowCursor(true);
	}

	void AcquireDevices()
	{
		mouseDevice->Acquire();
		keyboardDevice->Acquire();
	}

	void UnacquireDevices()
	{
		mouseDevice->Unacquire();
		keyboardDevice->Unacquire();
	}

	void Update()
	{
		AcquireDevices();
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

	bool MouseButtonUp(MouseButtons::BUTTON _btn)
	{
		if (currentState.rgbButtons[_btn] & 0x80)
		{
			mPressed[_btn] = true;
		}
		else
		{
			mPressed[_btn] = false;
		}

		if (!mPressed[_btn] && mPressedLastFrame[_btn])
		{
			mPressedLastFrame[_btn] = mPressed[_btn];
			return true;
		}

		mPressedLastFrame[_btn] = mPressed[_btn];
		return false;
	}

	bool MouseButtonDown(MouseButtons::BUTTON _btn)
	{
		if ((currentState.rgbButtons[_btn] & 0x80) && !mPressedLastFrame[_btn])
		{
			mPressedLastFrame[_btn] = true;
			return true;
		}
		else if (!(currentState.rgbButtons[_btn] & 0x80))
		{
			mPressedLastFrame[_btn] = false;
		}
		return false;
	}

	bool MouseButtonHeld(MouseButtons::BUTTON _btn)
	{
		if (currentState.rgbButtons[_btn] & 0x80)
			return true;
		return false;
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

