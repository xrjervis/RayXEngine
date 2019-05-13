#pragma once
struct KeyButtonState {
public:
	bool m_isKeyPressed = false;
	bool m_wasKeyJustPressed = false;
	bool m_wasKeyJustReleased = false;
};