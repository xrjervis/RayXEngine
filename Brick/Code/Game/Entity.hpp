#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"

class Entity {
public:
	Entity(float age, const Vector3& pos, const Rgba& color);
	~Entity();

	void Update(float ds);
	void Render() const;

public:
	float	m_age = 0.f;
	Vector3 m_position;
	Rgba	m_color;

	bool	m_isDead = false;
};