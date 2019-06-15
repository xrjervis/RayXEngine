#pragma once
#include "Engine/Math/Vector2.hpp"

class Entity2d {
public:
	Entity2d();
	~Entity2d();

	void Update(float ds);
	void Render() const;

public:
	Vector2		m_position;
	Vector2		m_velocity;
	Vector2		m_acceleration;
	Vector2		m_halfSize;
	float		m_isAlive = true;
	float		m_age = 0.f;
};