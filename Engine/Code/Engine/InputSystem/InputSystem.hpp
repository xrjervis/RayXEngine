#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Math/Vector2.hpp"
#include <string>

enum eMouseMode {
	MOUSEMODE_SNAP,
	MOUSEMODE_FREE,
};

class InputSystem{
public:
	~InputSystem(){}
	InputSystem();
	void BeginFrame();
	void EndFrame();

	void OnKeyPressed(u8 keyCode);
	void OnKeyReleased(u8 keyCode);
	void OnMousePressed(u8 keyCode);
	void OnMouseReleased(u8 keyCode);

	bool IsKeyPressed(u8 keyCode) const;
	bool WasKeyJustPressed(u8 keyCode) const;
	bool WasKeyJustReleased(u8 keyCode) const;

	bool IsMousePressed(u8 keyCode) const;
	bool WasMouseJustPressed(u8 keyCode) const;
	bool WasMouseJustReleased(u8 keyCode) const;

	u8	 GetKeyCodeFromName(const std::string& name) const;

	//Controller
	XboxController& GetController(int controllerID) { return m_controllers[controllerID]; }

	//Mouse
	eMouseMode GetMouseMode() const;
	void UpdateMouse();
	Vector2 GetMouseDelta() const;
	Vector2 GetMouseClientPos(void* handle) const;
	void SetMouseMode(eMouseMode mode);
	void ShowCursor(bool isShow);
	void LockCursor(bool isLock);

public:
	static const int	NUM_KEYS = 256;
	static const int	NUM_CONTROLLERS = 1;
	static const u8		KEYBOARD_ESCAPE;
	static const u8		KEYBOARD_BACKSPACE;
	static const u8		KEYBOARD_ENTER;
	static const u8		KEYBOARD_SPACE;
	static const u8		KEYBOARD_UP_ARROW;
	static const u8		KEYBOARD_LEFT_ARROW;
	static const u8		KEYBOARD_DOWN_ARROW;
	static const u8		KEYBOARD_RIGHT_ARROW;
	static const u8		KEYBOARD_SHIFT;
	static const u8		KEYBOARD_TAB;
	static const u8		KEYBOARD_CTRL;
	static const u8		KEYBOARD_DELETE;
	static const u8		KEYBOARD_HOME;
	static const u8		KEYBOARD_END;
	static const u8		KEYBOARD_PAGEUP;
	static const u8		KEYBOARD_PAGEDOWN;
	static const u8		KEYBOARD_F1;
	static const u8		KEYBOARD_F2;
	static const u8		KEYBOARD_F3;
	static const u8		KEYBOARD_F8;
	static const u8		KEYBOARD_F9;
	static const u8		KEYBOARD_F10;
	static const u8		KEYBOARD_A;
	static const u8		KEYBOARD_B;
	static const u8		KEYBOARD_C;
	static const u8		KEYBOARD_D;
	static const u8		KEYBOARD_E;
	static const u8		KEYBOARD_F;
	static const u8		KEYBOARD_G;
	static const u8		KEYBOARD_H;
	static const u8		KEYBOARD_I;
	static const u8		KEYBOARD_J;
	static const u8		KEYBOARD_K;
	static const u8		KEYBOARD_L;
	static const u8		KEYBOARD_M;
	static const u8		KEYBOARD_N;
	static const u8		KEYBOARD_O;
	static const u8		KEYBOARD_P;
	static const u8		KEYBOARD_Q;
	static const u8		KEYBOARD_R;
	static const u8		KEYBOARD_S;
	static const u8		KEYBOARD_T;
	static const u8		KEYBOARD_U;
	static const u8		KEYBOARD_V;
	static const u8		KEYBOARD_W;
	static const u8		KEYBOARD_X;
	static const u8		KEYBOARD_Y;
	static const u8		KEYBOARD_Z;
	static const u8		KEYBOARD_TILDE;
	static const u8		MOUSE_LEFT;
	static const u8		MOUSE_MIDDLE;
	static const u8		MOUSE_RIGHT;


protected:
	void UpdateKeyboard();
	void UpdateControllers();

	void SetMouseClientPos(const Vector2& pos);
	Vector2 GetCenterOfClientWindow() const;

protected:
	KeyButtonState m_keyStates[NUM_KEYS];
	KeyButtonState m_mouseStates[3];

	XboxController m_controllers[NUM_CONTROLLERS];

	Vector2 m_mousePosThisFrame = Vector2::ZERO;
	Vector2 m_mousePosLastFrame = Vector2::ZERO;
	eMouseMode m_mouseMode;
};