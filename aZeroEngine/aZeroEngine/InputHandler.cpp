#include "InputHandler.h"
#include <iostream>
//std::bitset<256> InputManager::keyset;
std::bitset<256> InputManager::keydownThisFrameSet;
std::bitset<256> InputManager::keydownLastFrameSet;
std::bitset<256> InputManager::keyupThisFrameSet;
Vector2 InputManager::clickedPosition = Vector2(0.f, 0.f);
Vector2 InputManager::cursorDirectionFrame = Vector2(0.f, 0.f);
Vector2 InputManager::lastCursorPosition = Vector2(0.f, 0.f);
bool InputManager::mouseMoved = false;
InputManager::MouseBtnState InputManager::mouseBtns[MOUSEBTN::MAX];

void InputManager::OnMouseBtnPressed(Vector2 _pos, MOUSEBTN _btn)
{
	if (!InputManager::mouseBtns[_btn].mouseDownLastFrame)
	{
		InputManager::mouseBtns[_btn].mouseDownThisFrame = true;
		InputManager::mouseBtns[_btn].mouseDownLastFrame = true;
	}

	clickedPosition = _pos;
	InputManager::mouseBtns[_btn].mouseDown = true;
}

void InputManager::OnMouseBtnReleased(Vector2 _pos, MOUSEBTN _btn)
{
	if (InputManager::mouseBtns[_btn].mouseDownLastFrame)
	{
		InputManager::mouseBtns[_btn].mouseDownThisFrame = false;
		InputManager::mouseBtns[_btn].mouseDownLastFrame = false;
	}

	InputManager::mouseBtns[_btn].mouseUpThisFrame = true;

	clickedPosition = _pos;
	InputManager::mouseBtns[_btn].mouseDown = false;
}

void InputManager::OnKeyPressed(unsigned char _keycode)
{
	bool down = InputManager::keydownLastFrameSet[_keycode];
	if (!down)
	{
		InputManager::keydownThisFrameSet[_keycode] = true;
		InputManager::keydownLastFrameSet[_keycode] = true;
	}

	keyset.set(_keycode, true);
}

void InputManager::OnKeyRelease(unsigned char _keycode)
{
	bool down = InputManager::keydownLastFrameSet[_keycode];
	if (down)
	{
		InputManager::keydownThisFrameSet[_keycode] = false;
		InputManager::keydownLastFrameSet[_keycode] = false;
	}

	InputManager::keyupThisFrameSet[_keycode] = true;

	keyset.set(_keycode, false);
}

void InputManager::OnMouseMove(LONG _xPos, LONG _yPos)
{
	cursorDirectionFrame.x = (float)_xPos;
	cursorDirectionFrame.y = (float)_yPos;
	mouseMoved = true;
}

void InputManager::OnMouseMove(Vector2 _pos)
{
	cursorDirectionFrame = _pos;
	mouseMoved = true;
}

void InputManager::EndFrame()
{
	mouseMoved = false;
	keydownThisFrameSet.reset();
	keyupThisFrameSet.reset();

	for (int i = 0; i < MOUSEBTN::MAX; i++)
	{
		mouseBtns[i].mouseUpThisFrame = false;
		mouseBtns[i].mouseDownThisFrame = false;
	}
}
