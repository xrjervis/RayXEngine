#pragma once
#include "Engine/Math/Vector2.hpp"

class Sprite;

class Entity2d {
public:
	Entity2d();
	virtual ~Entity2d();

	virtual void Update(float ds);

public:
	Vector2		m_position;
	Vector2		m_velocity;
	Vector2		m_acceleration;
	Vector2		m_halfSize;
	float		m_isAlive = true;
	float		m_age = 0.f;

	Sprite*		m_sprite = nullptr;
};