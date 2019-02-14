#define WIN32_LEAN_AND_MEAN		// Always #define this before #include <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <algorithm>
#include "Engine/Core/Window.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Window::Window(const std::wstring& title, eWindowMode mode, float fractionToDesktop, float clientAspect /*= 1.f*/) 
	: m_title(title) {
	RegisterWindowClass();
	ComputeDesktopSize();
	ComputeClientSizeFromAspect();
	CreateAndShowWindow();
	SetTitle(title);
	Adjust(mode, fractionToDesktop, clientAspect);
}

Window::Window(const std::wstring& title, float clientHeight, float clientAspect) 
	: m_title(title){
	RegisterWindowClass();
	ComputeDesktopSize();
	CreateAndShowWindow();
	SetTitle(title);
	Adjust(WINDOW_MODE_WINDOWED, clientHeight / m_desktopHeight, clientAspect);
}

Window::~Window() {
	::UnregisterClass(m_title.c_str(), ::GetModuleHandle(NULL));
}

void Window::ComputeDesktopSize() {
	// Always getting the primary monitor's resolution

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = ::GetDesktopWindow();
	::GetClientRect(desktopWindowHandle, &desktopRect);
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	m_desktopWidth = desktopWidth;
	m_desktopHeight = desktopHeight;
}

void Window::ComputeClientSizeFromAspect() {
	// Calculate maximum client size (as some % of desktop size)
	float clientWidth = m_desktopWidth * m_clientFractionToDesktop;
	float clientHeight = m_desktopHeight * m_clientFractionToDesktop;
	float desktopAspect = m_desktopWidth / m_desktopHeight;

	if (m_clientAspect > desktopAspect) {
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / m_clientAspect;
	}
	else {
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * m_clientAspect;
	}
	m_clientWidth = clientWidth;
	m_clientHeight = clientHeight;
}

void Window::RegisterWindowClass() {
	// Define a window style/class
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
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
	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (m_desktopWidth - m_clientWidth);
	float clientMarginY = 0.5f * (m_desktopHeight - m_clientHeight);
	RECT clientRect = {};
	clientRect.left = (int)clientMarginX;
	clientRect.right = clientRect.left + (int)m_clientWidth;
	clientRect.top = (int)clientMarginY;
	clientRect.bottom = clientRect.top + (int)m_clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	const DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	::AdjustWindowRectEx(&clientRect, windowStyleFlags, FALSE, windowStyleExFlags);
	m_hWnd = CreateWindowEx(
		windowStyleExFlags,
		m_title.c_str(),
		(LPCTSTR)m_title.c_str(),
		windowStyleFlags,
		clientRect.left,
		clientRect.top,
		(int)m_clientWidth,
		(int)m_clientHeight,
		nullptr,
		nullptr,
		(HINSTANCE)m_hInst,
		nullptr);
	if (!m_hWnd) {
		ERROR_AND_DIE("Create window class failed!");
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

void Window::SetTitle(const std::wstring& newTitle) {
	m_title = newTitle;
	::SetWindowText((HWND)m_hWnd, (LPCTSTR)m_title.c_str());
}

void Window::SetPosition(int topLeftX, int topLeftY) {
	::SetWindowPos((HWND)m_hWnd, nullptr, topLeftX, topLeftY, (int)m_clientWidth, (int)m_clientHeight, SWP_SHOWWINDOW);
}

void Window::Adjust(eWindowMode mode, float fractionToDesktop, float clientAspect /*= -1.f*/) {
	m_mode = mode;
	m_clientFractionToDesktop = fractionToDesktop;
	if(clientAspect >= 0.f){
		m_clientAspect = clientAspect;
	}
	switch (m_mode) {
	case WINDOW_MODE_WINDOWED:
		m_windowStyleFlags = WS_OVERLAPPEDWINDOW;
		break;
	case WINDOW_MODE_BORDERLESS_FULLSCREEN:
		m_clientFractionToDesktop = 1.f;
		m_clientAspect = m_desktopWidth / m_desktopHeight;
		m_windowStyleFlags = WS_POPUP;
		break;
	}

	ComputeClientSizeFromAspect();

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (m_desktopWidth - m_clientWidth);
	float clientMarginY = 0.5f * (m_desktopHeight - m_clientHeight);

	::SetWindowLong((HWND)m_hWnd, GWL_STYLE, m_windowStyleFlags);
	SetPosition((int)clientMarginX, (int)clientMarginY);
}