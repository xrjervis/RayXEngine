#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba.hpp"
#include <string>
#include <memory>

class Texture2D;

class Sprite {
public:
	Sprite();
	Sprite(Texture2D* texture, const AABB2& uv = AABB2());
	~Sprite() = default;

	void SetTexture(Texture2D* texture);
	void SetBillboard(bool useBillboard);
	void SetSize(float width, float height);
	void SetPivot(float pivotX, float pivotY);
	void SetUV(const AABB2& uv);
	void SetTint(const Rgba& tint);

public:
	Texture2D*	m_texture = nullptr;
	AABB2		m_uv;
	Vector2		m_pivot = Vector2(.5f, .5f);
	Rgba		m_tint = Rgba::WHITE;
	bool		m_useBillboard = false;
	float		m_width = 1.f;
	float		m_height = 1.f;
};