#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Window.hpp"

const u8 InputSystem::KEYBOARD_ESCAPE =			VK_ESCAPE;
const u8 InputSystem::KEYBOARD_BACKSPACE =		VK_BACK;
const u8 InputSystem::KEYBOARD_ENTER =			VK_RETURN;
const u8 InputSystem::KEYBOARD_TILDE =			VK_OEM_3; //'`~'
const u8 InputSystem::KEYBOARD_SPACE =			VK_SPACE;
const u8 InputSystem::KEYBOARD_UP_ARROW =		VK_UP;
const u8 InputSystem::KEYBOARD_LEFT_ARROW =		VK_LEFT;
const u8 InputSystem::KEYBOARD_DOWN_ARROW =		VK_DOWN;
const u8 InputSystem::KEYBOARD_RIGHT_ARROW =	VK_RIGHT;
const u8 InputSystem::KEYBOARD_SHIFT =			VK_SHIFT;
const u8 InputSystem::KEYBOARD_TAB =			VK_TAB;
const u8 InputSystem::KEYBOARD_CTRL =			VK_CONTROL;
const u8 InputSystem::KEYBOARD_DELETE =			VK_DELETE;
const u8 InputSystem::KEYBOARD_HOME =			VK_HOME;
const u8 InputSystem::KEYBOARD_END =			VK_END;
const u8 InputSystem::KEYBOARD_PAGEUP =			VK_PRIOR;
const u8 InputSystem::KEYBOARD_PAGEDOWN =		VK_NEXT;
const u8 InputSystem::KEYBOARD_F1 =				VK_F1;
const u8 InputSystem::KEYBOARD_F2 =				VK_F2;
const u8 InputSystem::KEYBOARD_F3 =				VK_F3;
const u8 InputSystem::KEYBOARD_F8 =				VK_F8;
const u8 InputSystem::KEYBOARD_F9 =				VK_F9;
const u8 InputSystem::KEYBOARD_F10 =			VK_F10;
const u8 InputSystem::KEYBOARD_A =				'A';
const u8 InputSystem::KEYBOARD_B =				'B';
const u8 InputSystem::KEYBOARD_C =				'C';
const u8 InputSystem::KEYBOARD_D =				'D';
const u8 InputSystem::KEYBOARD_E =				'E';
const u8 InputSystem::KEYBOARD_F =				'F';
const u8 InputSystem::KEYBOARD_G =				'G';
const u8 InputSystem::KEYBOARD_H =				'H';
const u8 InputSystem::KEYBOARD_I =				'I';
const u8 InputSystem::KEYBOARD_J =				'J';
const u8 InputSystem::KEYBOARD_K =				'K';
const u8 InputSystem::KEYBOARD_L =				'L';
const u8 InputSystem::KEYBOARD_M =				'M';
const u8 InputSystem::KEYBOARD_N =				'N';
const u8 InputSystem::KEYBOARD_O =				'O';
const u8 InputSystem::KEYBOARD_P =				'P';
const u8 InputSystem::KEYBOARD_Q =				'Q';
const u8 InputSystem::KEYBOARD_R =				'R';
const u8 InputSystem::KEYBOARD_S =				'S';
const u8 InputSystem::KEYBOARD_T =				'T';
const u8 InputSystem::KEYBOARD_U =				'U';
const u8 InputSystem::KEYBOARD_V =				'V';
const u8 InputSystem::KEYBOARD_W =				'W';
const u8 InputSystem::KEYBOARD_X =				'X';
const u8 InputSystem::KEYBOARD_Y =				'Y';
const u8 InputSystem::KEYBOARD_Z =				'Z';
const u8 InputSystem::MOUSE_LEFT =				VK_LBUTTON;
const u8 InputSystem::MOUSE_RIGHT =				VK_RBUTTON;
const u8 InputSystem::MOUSE_MIDDLE =			VK_SHIFT;


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
void RunMessagePump() {
	MSG queuedMessage;
	for (;; ) {
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, nullptr, 0, 0, PM_REMOVE);
		if (!wasMessagePresent) {
			break;
		}
		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" function
	}
}

InputSystem::InputSystem() {
	SetMouseMode(MOUSEMODE_FREE);
}

void InputSystem::BeginFrame() {
	UpdateMouse();
	UpdateControllers();
	UpdateKeyboard();
	RunMessagePump();
}

void InputSystem::EndFrame() {
}

void InputSystem::OnKeyPressed(u8 keyCode) {
	if (m_keyStates[keyCode].m_isKeyPressed == false) {
		m_keyStates[keyCode].m_isKeyPressed = true;
		m_keyStates[keyCode].m_wasKeyJustPressed = true;
	}
}

void InputSystem::OnKeyReleased(u8 keyCode) {
	// left & right button release
	if (keyCode == 0U) {
		OnKeyReleased(InputSystem::MOUSE_LEFT);
		OnKeyReleased(InputSystem::MOUSE_RIGHT);
	}
	m_keyStates[keyCode].m_isKeyPressed = false;
	m_keyStates[keyCode].m_wasKeyJustReleased = true;
}

bool InputSystem::IsKeyPressed(u8 keyCode) const {
	return m_keyStates[keyCode].m_isKeyPressed;
}

bool InputSystem::WasKeyJustPressed(u8 keyCode) const {
	return m_keyStates[keyCode].m_wasKeyJustPressed;
}

bool InputSystem::WasKeyJustReleased(u8 keyCode) const {
	return m_keyStates[keyCode].m_wasKeyJustReleased;
}
 
u8 InputSystem::GetKeyCodeFromName(const std::string& name) const {
	if (name == "A")	return KEYBOARD_A;
	if (name == "B")	return KEYBOARD_B;
	if (name == "C")	return KEYBOARD_C;
	if (name == "D")	return KEYBOARD_D;
	if (name == "E")	return KEYBOARD_E;
	if (name == "F")	return KEYBOARD_F;
	if (name == "G")	return KEYBOARD_G;
	if (name == "H")	return KEYBOARD_H;
	if (name == "I")	return KEYBOARD_I;
	if (name == "J")	return KEYBOARD_J;
	if (name == "K")	return KEYBOARD_K;
	if (name == "L")	return KEYBOARD_L;
	if (name == "M")	return KEYBOARD_M;
	if (name == "N")	return KEYBOARD_N;
	if (name == "O")	return KEYBOARD_O;
	if (name == "P")	return KEYBOARD_P;
	if (name == "Q")	return KEYBOARD_Q;
	if (name == "R")	return KEYBOARD_R;
	if (name == "S")	return KEYBOARD_S;
	if (name == "T")	return KEYBOARD_T;
	if (name == "U")	return KEYBOARD_U;
	if (name == "V")	return KEYBOARD_V;
	if (name == "W")	return KEYBOARD_W;
	if (name == "X")	return KEYBOARD_X;
	if (name == "Y")	return KEYBOARD_Y;
	if (name == "Z")	return KEYBOARD_Z;
	if (name == "Space")return KEYBOARD_SPACE;
	if (name == "Enter")return KEYBOARD_ENTER;

	return 0;
}

Vector2 InputSystem::GetMouseDelta() const{
	return m_mousePosThisFrame - m_mousePosLastFrame;
}

Vector2 InputSystem::GetMouseClientPos() const {
	POINT desktopPos;
	::GetCursorPos(&desktopPos);
	HWND hwnd = (HWND)::GetActiveWindow();
	::ScreenToClient(hwnd, &desktopPos);
	POINT clientPos = desktopPos;

	return Vector2(clientPos.x, clientPos.y);
}

void InputSystem::SetMouseMode(eMouseMode mode) {
	m_mouseMode = mode;
	m_mousePosLastFrame = GetCenterOfClientWindow();
	SetMouseClientPos(m_mousePosLastFrame);
	m_mousePosThisFrame = m_mousePosLastFrame;

	switch (mode) {
	case MOUSEMODE_FREE:
		ShowCursor(true);
		LockCursor(false);
		break;
	case MOUSEMODE_SNAP:
		ShowCursor(false);
		LockCursor(true);
		break;
	default:
		ShowCursor(true);
		LockCursor(false);
		break;
	}
}

void InputSystem::ShowCursor(bool isShow) {
	if (isShow == true) {
		while (::ShowCursor(isShow) <= 0);
	}
	else {
		while (::ShowCursor(isShow) >= 0);
	}
}

void InputSystem::LockCursor(bool isLock) {
	if (!isLock) {
		::ClipCursor(nullptr); // this unlock the mouse
	}
	else {
		HWND hwnd = (HWND)::GetActiveWindow(); // Get your windows HWND

		RECT clientRect; // window class RECT
		::GetClientRect(hwnd, &clientRect);

		POINT offset;
		offset.x = 0;
		offset.y = 0;
		::ClientToScreen(hwnd, &offset);

		clientRect.left += offset.x;
		clientRect.right += offset.x;
		clientRect.top += offset.y;
		clientRect.bottom += offset.y;

		::ClipCursor(&clientRect);
	}
}

void InputSystem::UpdateKeyboard() {
	for(int keyCode = 0; keyCode < NUM_KEYS; ++keyCode){
		m_keyStates[keyCode].m_wasKeyJustPressed = false;
		m_keyStates[keyCode].m_wasKeyJustReleased = false;
	}
}

void InputSystem::UpdateControllers() {
	for (int i = 0; i < NUM_CONTROLLERS; ++i) {
		m_controllers[i].UpdateXboxControllerState(i);
	}
}

eMouseMode InputSystem::GetMouseMode() const {
	return m_mouseMode;
}

void InputSystem::UpdateMouse() {
	m_mousePosLastFrame = m_mousePosThisFrame;
	m_mousePosThisFrame = GetMouseClientPos();

	if (m_mouseMode == MOUSEMODE_SNAP) {
		m_mousePosLastFrame = GetCenterOfClientWindow();
		SetMouseClientPos(m_mousePosLastFrame);
	}
}

void InputSystem::SetMouseClientPos(const Vector2& pos) {
	POINT clientPos;
	clientPos.x = (LONG)pos.x;
	clientPos.y = (LONG)pos.y;

	HWND hwnd = (HWND)::GetActiveWindow();
	::ClientToScreen(hwnd, &clientPos);
	POINT desktopPos = clientPos;
	::SetCursorPos(desktopPos.x, desktopPos.y);
}

Vector2 InputSystem::GetCenterOfClientWindow() const {
	HWND hwnd = (HWND)::GetActiveWindow();
	RECT clientRect; // window class RECT
	::GetClientRect(hwnd, &clientRect);

	POINT clientPos;
	clientPos.x = clientRect.right / 2L;
	clientPos.y = clientRect.bottom / 2L;

	return Vector2(clientPos.x, clientPos.y);
}
