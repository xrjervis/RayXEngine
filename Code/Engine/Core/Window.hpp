#pragma once
#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <memory>
#include <wtypes.h>

using WindowMessage_cb = bool(*)(unsigned int msg, size_t wparam, size_t lparam);

enum eWindowMode {
	WINDOW_MODE_WINDOWED,
	WINDOW_MODE_BORDERLESS_FULLSCREEN,
	WINDOW_MODE_FULLSCREEN,
	NUM_WINDOW_MODE
};

class Window {
public:
	Window(const std::wstring& title, float clientHeight, float clientAspect);
	Window(const std::wstring& title, eWindowMode mode, float fractionToDesktop, float clientAspect = 1.f);
	~Window();

	void RegisterHandler(WindowMessage_cb cb);
	void UnregisterHandler(WindowMessage_cb cb);

	void* GetHandle() const { return m_hWnd; }
	void* GetInstance() const { return m_hInst; }
	float GetDesktopWidth() const { return m_desktopWidth; }
	float GetDesktopHeight() const { return m_desktopHeight; }
	float GetClientWidth() const { return m_clientWidth; }
	float GetClientHeight() const { return m_clientHeight; }
	AABB2 GetOrtho() const { return AABB2(Vector2(0, 0), Vector2(m_clientWidth, m_clientHeight)); }

	void SetTitle(const std::wstring& newTitle);
	void SetPosition(int topLeftX, int topLeftY);
	void Adjust(eWindowMode mode, float fractionToDesktop, float clientAspect = -1.f);
	

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
	void ComputeDesktopSize();
	void ComputeClientSizeFromAspect();
	void RegisterWindowClass();
	void CreateAndShowWindow();

private:
	eWindowMode		m_mode = WINDOW_MODE_WINDOWED;
	float			m_clientFractionToDesktop = 0.f;
	float			m_clientWidth;
	float			m_clientHeight;
	float			m_clientAspect = 1.f;
	float			m_desktopWidth;
	float			m_desktopHeight;
	std::wstring	m_title = L"Untitled";
	void*			m_hInst;
	void*			m_hWnd;

	DWORD			m_windowStyleFlags = WS_OVERLAPPEDWINDOW;
	DWORD			m_windowStyleExFlags = WS_EX_APPWINDOW;


public:
	std::vector<WindowMessage_cb> listeners;
};