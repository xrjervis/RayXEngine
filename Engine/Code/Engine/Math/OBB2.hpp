#pragma once
#include "Engine/Math/Vector2.hpp"


class OBB2 {
public:
	OBB2();
	explicit OBB2(const Vector2& center, const Vector2& baseX, const Vector2& baseY, const Vector2& halfExtensions);
	explicit OBB2(const Vector2& center, const Vector2& baseX, const Vector2& baseY, float halfExtensionX, float halfExtensionY);
	explicit OBB2(const Vector2& center, const Vector2& halfExtensions);
	explicit OBB2(const Vector2& center, float halfExtensionX, float halfExtensionY);
	~OBB2();

	float	GetOrient() const;
	void	SetOrient(float degrees);

public:
	Vector2 m_center;
	Vector2 m_baseX;
	Vector2 m_baseY;
	float	m_halfExtensionX;
	float	m_halfExtensionY;
};