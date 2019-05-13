#pragma once
#include "Engine/Math/Vector3.hpp"

class Vector4 {
public:
	~Vector4() = default;											// destructor: do nothing (for speed)
	Vector4() = default;											// default constructor: do nothing (for speed)

	explicit Vector4(float initialX, float initialY, float initialZ, float initialW); // explicit constructor 
	explicit Vector4(int initialX, int initialY, int initialZ, int initialW);
	explicit Vector4(const Vector3& xyz, float initW);

	Vector3 xyz() const;

public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	static Vector4 ZERO;
	static Vector4 ONE;
};