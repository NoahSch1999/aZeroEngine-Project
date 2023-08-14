#pragma once
#include <Windows.h>
#include <bitset>
#include "VertexDefinitions.h"

// TODO: REWORK the whole class

/** @brief Enumeration for the mouse buttons supported.
* The last enum "MAX" is there to specify the max amount of enums supported.
*/
enum MOUSEBTN { LEFT, RIGHT, MIDDLE, MAX };

/** @brief Singleton class for managing user input.
*/
class InputManager
{
private:
	static DXM::Vector2 clickedPosition;

	struct MouseBtnState
	{
		bool mouseDown = false;
		bool mouseUpThisFrame = false;
		bool mouseDownThisFrame = false;
		bool mouseDownLastFrame = false;
	};

	static MouseBtnState mouseBtns[MOUSEBTN::MAX];
	static bool mouseMoved;

	inline static std::bitset<256>keyset;

	static std::bitset<256>keydownThisFrameSet;
	static std::bitset<256>keydownLastFrameSet;

	static std::bitset<256>keyupThisFrameSet;

	static void OnMouseBtnPressed(DXM::Vector2 _pos, MOUSEBTN _btn);

	static void OnMouseBtnReleased(DXM::Vector2 _pos, MOUSEBTN _btn);

	static void OnKeyPressed(unsigned char _keycode);

	static void OnKeyRelease(unsigned char _keycode);

	friend LRESULT CALLBACK WndProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

	static DXM::Vector2 lastCursorPosition;
	static DXM::Vector2 cursorDirectionFrame;
	static void OnMouseMove(LONG _xPos, LONG _yPos);
	static void OnMouseMove(DXM::Vector2 _pos);

public:

	/** Checks whether or not the mouse moved this frame.
	@return static bool TRUE: Mouse moved this frame, FALSE:Mouse didn't move this frame.
	*/
	static bool MouseMoved() { return mouseMoved; }

	/** Returns the direction in screen pixels that the mouse moved this frame.
	* x > 0 means it moved right
	* x < 0 means it moved left
	* y > 0 means it moved down
	* y < 0 means it moved up
	@return Vector2 The direction in screen pixels that the mouse moved this frame.
	*/
	static DXM::Vector2 GetMouseFrameDirection() { return cursorDirectionFrame; }

	/** Checks whether or not the specified mouse button is currently down.
	@param _btn MOUSEBTN enumeration for the mouse button to check.
	@return static bool TRUE: Button is down, FALSE: Button isn't down.
	*/
	static bool MouseBtnHeld(MOUSEBTN _btn) { return mouseBtns[_btn].mouseDown; }

	/** Checks whether or not the specified mouse button has been pressed down this frame.
	@param _btn MOUSEBTN enumeration for the mouse button to check.
	@return static bool TRUE: Button has been pressed down this frame, FALSE: Button hasn't been pressed down this frame.
	*/
	static bool MouseBtnDown(MOUSEBTN _btn) { return mouseBtns[_btn].mouseDownThisFrame; }

	/** Checks whether or not the specified mouse button has been let go this frame.
	@param _btn MOUSEBTN enumeration for the mouse button to check.
	@return static bool TRUE: Button has been let go this frame, FALSE: Button hasn't been let go this frame.
	*/
	static bool MouseBtnUp(MOUSEBTN _btn) { return mouseBtns[_btn].mouseUpThisFrame; }

	/** Checks whether or not the specified key is currently down.
	@param _keycode Virtual Key code for the key to check.
	@return static bool TRUE: Key is down, FALSE: Key isn't down.
	*/
	static bool KeyHeld(unsigned char _keycode) { return keyset[_keycode]; }

	/** Checks whether or not the specified key has been pressed down this frame.
	@param _keycode Virtual Key code for the key to check.
	@return static bool TRUE: Key has been pressed down this frame, FALSE: Key hasn't been pressed down this frame.
	*/
	static bool KeyDown(unsigned char _keycode) { return keydownThisFrameSet[_keycode]; }

	/** Checks whether or not the specified key has been let go this frame.
	@param _keycode Virtual Key code for the key to check.
	@return static bool TRUE: Key has been let go this frame, FALSE: Key hasn't been let go this frame.
	*/
	static bool KeyUp(unsigned char _keycode) { return keyupThisFrameSet[_keycode]; }

	/** Returns the cursor screen position when last pressed down this frame.
	@return Vector2 The X and Y positions in screen coordinates.
	*/
	static DXM::Vector2 GetClickedPosition() { return clickedPosition; }

	/** Specifies that the current frame has ended.
	* Should only be called ONCE right before the frame will end.
	@return void
	*/
	static void EndFrame(); // make friend to hide from api
};
