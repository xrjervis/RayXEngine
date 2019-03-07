#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Core/type.hpp"
#include <sstream>

class Rgba {
public:
	~Rgba() = default;
	explicit Rgba();
	explicit Rgba(int redByte, int greenByte, int blueByte, int alphaByte = 255);
	explicit Rgba(u8 redByte, u8 greenByte, u8 blueByte, u8 alphaByte = 255u);
	explicit Rgba(float red, float green, float blue, float alpha = 1.0f);
	Rgba(const u32 hexByte);

	void SetAsBytes(u8 redByte, u8 greenByte, u8 blueByte, u8 alphaByte = 255);
	void SetAsFloats(float normalizedRed, float normalizedGreen, float normalizedBlue, float normalizedAlpha = 1.0f);
	Vector4 GetAsFloats() const;
	Rgba ScaleRGB(float rgbScale);
	void ScaleAlpha(float alphaScale);
	void SetFromText(const char* text);
	bool RgbEqual(const Rgba& compare);
	float GetFloatR() const;
	float GetFloatG() const;
	float GetFloatB() const;
	float GetFloatA() const;
	Vector3 GetFloatRGB() const;

public:
	u8 r;
	u8 g;
	u8 b;
	u8 a;

	static const u32 WHITE;
	static const u32 BLACK;
	static const u32 RED;
	static const u32 GREEN;
	static const u32 BLUE;
	static const u32 YELLOW;
	static const u32 GRAY;
	static const u32 MIDNIGHTBLUE;
	static const u32 PURPLE;
};

Rgba Interpolate(const Rgba& start, const Rgba& end, float fractionTowardEnd);
Rgba MoveTowards(const Rgba& current, const Rgba& target, float delta);

bool operator>>(std::stringstream& ss, Rgba& toValue);
