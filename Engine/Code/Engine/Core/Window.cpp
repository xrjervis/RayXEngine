#define WIN32_LEAN_AND_MEAN		// Always #define this before #include <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <algorithm>
#include <windowsx.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "Engine/Core/Window.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//https://github.com/melak47/BorderlessWindow/blob/master/BorderlessWindow/src/BorderlessWindow.cpp
// we cannot just use WS_POPUP style
// WS_THICKFRAME: without this the window cannot be resized and so aero snap, de-maximizing and minimizing won't work
// WS_SYSMENU: enables the context menu with the move, close, maximize, minize... commands (shift + right-click on the task bar item)
// WS_CAPTION: enables aero minimize animation/transition
// WS_MAXIMIZEBOX, WS_MINIMIZEBOX: enable minimize/maximize
enum class Style : DWORD {
	WINDOWED = WS_OVERLAPPEDWINDOW | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
	BORDERLESS = WS_POPUP
};

Window::Window(const std::wstring& title, eWindowMode mode, float aspect) 
	: m_title(title)
	, m_mode(mode)
	, m_aspect(aspect){

	RegisterWindowClass();
	CreateAndShowWindow();
}

Window::~Window() {
	::UnregisterClass(m_title.c_str(), ::GetModuleHandle(NULL));
}

void Window::RegisterWindowClass() {
	// Define a window style/class
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	// A window must belong to a window class that has the CS_DBLCLKS class style
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; 
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = (HINSTANCE)m_hInst;
	wcex.hIcon = ::LoadIcon((HINSTANCE)m_hInst, (LPCTSTR)IDI_WINLOGO);
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = m_title.c_str();
	wcex.hIconSm = ::LoadIcon((HINSTANCE)m_hInst, (LPCTSTR)IDI_WINLOGO);
	if(!::RegisterClassEx(&wcex)){
		ERROR_AND_DIE("Register window class failed!");
	}
}

void Window::CreateAndShowWindow() {
	RECT desktopRect;
	HWND desktopWindowHandle = ::GetDesktopWindow();
	::GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);

	float fractionToDesktop;
	if (m_mode == WINDOW_MODE_WINDOWED) {
		fractionToDesktop = 0.9f;
	}
	else {
		fractionToDesktop = 1.0f;
	}
	float clientHeight = desktopHeight * fractionToDesktop;
	float clientWidth = clientHeight * m_aspect;

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect = {};
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	if (m_mode == WINDOW_MODE_WINDOWED) {
		windowStyleFlags = WS_OVERLAPPEDWINDOW;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}
	else {
		windowStyleFlags = WS_POPUP;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}
	::AdjustWindowRectEx(&clientRect, windowStyleFlags, FALSE, windowStyleExFlags);
	m_hWnd = CreateWindowEx(
		windowStyleExFlags,
		m_title.c_str(),
		(LPCTSTR)m_title.c_str(),
		windowStyleFlags,
		clientRect.left,
		clientRect.top,
		(int)clientWidth,
		(int)clientHeight,
		nullptr,
		nullptr,
		(HINSTANCE)m_hInst,
		nullptr);
	if (!m_hWnd) {
		ERROR_AND_DIE("Create window class failed!");
	}
	if (m_mode == WINDOW_MODE_BORDERLESS_FULLSCREEN) {
		::SetWindowLongPtr((HWND)m_hWnd, GWL_STYLE, static_cast<LONG>(Style::BORDERLESS));
	}
	::SetWindowLongPtr((HWND)m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::ShowWindow((HWND)m_hWnd, SW_SHOW);
	::UpdateWindow((HWND)m_hWnd);
}

void Window::RegisterHandler(WindowMessage_cb cb) {
	listeners.push_back(cb);
}

void Window::UnregisterHandler(WindowMessage_cb cb) {
	listeners.erase(std::remove(listeners.begin(), listeners.end(), cb), listeners.end());
}

float Window::GetClientWidth() const {
	RECT clientRect;
	::GetClientRect((HWND)m_hWnd, &clientRect);
	return (float)(clientRect.right - clientRect.left);
}

float Window::GetClientHeight() const {
	RECT clientRect;
	::GetClientRect((HWND)m_hWnd, &clientRect);
	return (float)(clientRect.bottom - clientRect.top);
}

void Window::AdjustPositionAndSize(const Vector2& newPos, const Vector2& newSize) {
	RECT clientRect = {};
	clientRect.left = (int)newPos.x;
	clientRect.right = clientRect.left + (int)newSize.x;
	clientRect.top = (int)newPos.y;
	clientRect.bottom = clientRect.top + (int)newSize.y;
	DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW;
	DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	if (m_mode == WINDOW_MODE_WINDOWED) {
		windowStyleFlags = WS_OVERLAPPEDWINDOW;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}
	else {
		windowStyleFlags = WS_POPUP;
		windowStyleExFlags = WS_EX_APPWINDOW;
	}
	::AdjustWindowRectEx(&clientRect, windowStyleFlags, FALSE, windowStyleExFlags);
	::SetWindowPos((HWND)m_hWnd, nullptr, clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
	::UpdateWindow((HWND)m_hWnd);
}

void Window::SetTitle(const std::wstring& wstr) {
	::SetWindowTextW((HWND)m_hWnd, wstr.c_str());
}
