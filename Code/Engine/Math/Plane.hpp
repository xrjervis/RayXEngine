#pragma once
#include "Engine/Math/Vector3.hpp"

class Plane {
public:
	Plane() = default;
	Plane(const Vector3& position, const Vector3& normal);
	~Plane() = default;

public:
	Vector3 m_position;
	Vector3 m_normal;
};