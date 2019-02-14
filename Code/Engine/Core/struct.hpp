#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/Rgba.hpp"
#include <string>

struct ConsoleString {
	ConsoleString() = default;
	ConsoleString(std::string str, const Rgba& color, ConsoleString* cstr = nullptr)
		:m_str(str), m_color(color), m_cstrInfo(cstr) {}
	~ConsoleString() { if (m_cstrInfo) m_cstrInfo; m_cstrInfo = nullptr; }

	std::string m_str;
	Rgba m_color;
	ConsoleString* m_cstrInfo = nullptr;
};