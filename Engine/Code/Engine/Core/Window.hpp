#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <memory>
#include <wtypes.h>

using WindowMessage_cb = bool(*)(unsigned int msg, size_t wparam, size_t lparam);

enum eWindowMode {
	WINDOW_MODE_WINDOWED,
	WINDOW_MODE_BORDERLESS_FULLSCREEN,
	NUM_WINDOW_MODE
};

class Window {
public:
	Window(const std::wstring& title, eWindowMode mode, float aspect);
	~Window();

	void RegisterHandler(WindowMessage_cb cb);
	void UnregisterHandler(WindowMessage_cb cb);

	void* GetHandle() const { return m_hWnd; }
	void* GetInstance() const { return m_hInst; }

	float GetClientWidth() const;
	float GetClientHeight() const;

	void AdjustPositionAndSize(const Vector2& newPos, const Vector2& newSize);

	void SetTitle(const std::wstring& wstr);

	static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
		Window* currentWindow = reinterpret_cast<Window*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if(currentWindow && !currentWindow->listeners.empty()){
			for (int i = 0; i < currentWindow->listeners.size(); ++i) {
				if (currentWindow->listeners[i](msg, wparam, lparam) == false) {
					return 0;
				}
			}
		}
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}

private:
	void RegisterWindowClass();
	void CreateAndShowWindow();

private:
	eWindowMode		m_mode = WINDOW_MODE_WINDOWED;
	std::wstring	m_title = L"Untitled";
	float			m_aspect;
	void*			m_hInst;
	void*			m_hWnd;

public:
	std::vector<WindowMessage_cb> listeners;
};