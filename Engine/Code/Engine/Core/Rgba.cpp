#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <vector>
#include <string>

const u32 Rgba::WHITE = 0xFFFFFFFF;
const u32 Rgba::BLACK = 0x000000FF;
const u32 Rgba::RED = 0xFF0000FF;
const u32 Rgba::GREEN = 0x00FF00FF;
const u32 Rgba::BLUE = 0x0000FFFF;
const u32 Rgba::YELLOW = 0xFFFF00FF;
const u32 Rgba::GRAY = 0x808080FF;
const u32 Rgba::MIDNIGHTBLUE = 0x003366FF;
const u32 Rgba::PURPLE = 0xFF00FFFF;
const u32 Rgba::CYAN = 0x00FFFFFF;


Rgba::Rgba() : r(0), g(0), b(0), a(255) {}

Rgba::Rgba(u8 redByte, u8 greenByte, u8 blueByte, u8 alphaByte) {
	SetAsBytes(redByte, greenByte, blueByte, alphaByte);
}


Rgba::Rgba(const u32 hexByte) {
	u32 temp = hexByte;
	a = (u8)(temp & 0xff);
	temp >>= 8;
	b = (u8)(temp & 0xff);
	temp >>= 8;
	g = (u8)(temp & 0xff);
	temp >>= 8;
	r = (u8)(temp & 0xff);
}

Rgba::Rgba(float red, float green, float blue, float alpha /*= 1.0f*/) {
	SetAsFloats(red, green, blue, alpha);
}


Rgba::Rgba(int redByte, int greenByte, int blueByte, int alphaByte /*= 255u*/) {
	r = static_cast<unsigned char>(redByte);
	g = static_cast<unsigned char>(greenByte);
	b = static_cast<unsigned char>(blueByte);
	a = static_cast<unsigned char>(alphaByte);
}

void Rgba::SetAsBytes(u8 redByte, u8 greenByte, u8 blueByte, u8 alphaByte) {
	r = redByte;
	g = greenByte;
	b = blueByte;
	a = alphaByte;
}

void Rgba::SetAsFloats(float red, float green, float blue, float alpha) {
	red = Clamp01(red);
	green = Clamp01(green);
	blue = Clamp01(blue);
	alpha = Clamp01(alpha);
	r = static_cast<unsigned char>(red * 255.f);
	g = static_cast<unsigned char>(green * 255.f);
	b = static_cast<unsigned char>(blue * 255.f);
	a = static_cast<unsigned char>(alpha * 255.f);
}

Vector4 Rgba::GetAsFloats() const {
	Vector4 vec4(
		static_cast<float>(r) / 255.f,
		static_cast<float>(g) / 255.f,
		static_cast<float>(b) / 255.f,
		static_cast<float>(a) / 255.f
	);
	return vec4;
}

Rgba Rgba::ScaleRGB(float rgbScale) {
	r = static_cast<unsigned char>(ClampFloat(static_cast<float>(r) * rgbScale, 0.f, 255.f));
	g = static_cast<unsigned char>(ClampFloat(static_cast<float>(g) * rgbScale, 0.f, 255.f));
	b = static_cast<unsigned char>(ClampFloat(static_cast<float>(b) * rgbScale, 0.f, 255.f));
	return *this;
}

void Rgba::ScaleAlpha(float alphaScale) {
	a = static_cast<unsigned char>(ClampFloat(static_cast<float>(a) * alphaScale, 0.f, 255.f));
}

void Rgba::SetFromText(const char* text) {
	std::vector<unsigned char> elements;

	char* end = nullptr;
	while (text[0] != '\0') {
		while (text[0] == ' ' || text[0] == ',') text++;
		if (text[0] != '\0') {
			unsigned char element = static_cast<unsigned char>(ClampInt(std::strtol(text, &end, 10), 0, 255));
			elements.push_back(element);
			text = end;
		}
	}

	if (elements.size() < 3 || elements.size() > 4) {
		return;
	}

	if (elements.size() >= 3) {
		r = elements[0];
		g = elements[1];
		b = elements[2];
	}

	if (elements.size() == 4) {
		a = elements[3];
	}

}

bool Rgba::RgbEqual(const Rgba& compare) {
	if (compare.r == r && compare.g == g && compare.b == b) {
		return true;
	}
	else {
		return false;
	}
}

float Rgba::GetFloatR() const {
	return RangeMapFloat((float)r, 0.f, 255.f, 0.f, 1.f);
}

float Rgba::GetFloatG() const {
	return RangeMapFloat((float)g, 0.f, 255.f, 0.f, 1.f);
}

float Rgba::GetFloatB() const {
	return RangeMapFloat((float)b, 0.f, 255.f, 0.f, 1.f);
}

float Rgba::GetFloatA() const {
	return RangeMapFloat((float)a, 0.f, 255.f, 0.f, 1.f);
}

Vector3 Rgba::GetFloatRGB() const {
	return Vector3(GetFloatR(), GetFloatG(), GetFloatB());
}

Rgba Interpolate(const Rgba& start, const Rgba& end, float fractionTowardEnd) {
	return Rgba(
		Interpolate(start.r, end.r, fractionTowardEnd),
		Interpolate(start.g, end.g, fractionTowardEnd),
		Interpolate(start.b, end.b, fractionTowardEnd),
		Interpolate(start.a, end.a, fractionTowardEnd)
	);
}

Rgba MoveTowards(const Rgba& current, const Rgba& target, float delta) {
	return Rgba(
		(unsigned char)MoveTowards(current.r, target.r, delta),
		(unsigned char)MoveTowards(current.g, target.g, delta),
		(unsigned char)MoveTowards(current.b, target.b, delta),
		(unsigned char)MoveTowards(current.a, target.a, delta)
	);
}

bool operator>>(std::stringstream& ss, Rgba& toValue) {
	std::string rawString;

	ss >> rawString;
	int length = (int)rawString.length();
	if (rawString[0] == '(' && rawString[length - 1] == ')') {
		rawString = rawString.substr(1, length - 2);
	}
	toValue.SetFromText(rawString.c_str());
	return true;
}
