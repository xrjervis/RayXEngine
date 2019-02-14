#pragma once
#include "Engine/Math/Vector3.hpp"

class Sphere {
public:
	Sphere() = default;
	Sphere(const Vector3& center, float radius);
	~Sphere() = default;

	bool IsPointInside(const Vector3& point) const;

public:
	Vector3 m_center;
	float m_radius;
};