#pragma once
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <string>

// SpriteCoords(0, 0) is at top left, just like UV
class SpriteSheet{
public:
	SpriteSheet(Texture2D* texture, const IntVector2& layout);
	~SpriteSheet() = default;

	AABB2		GetUVFromIndex(int idx) const;
	AABB2		GetUVFromSpriteCoords(int x, int y) const;
	AABB2		GetUVFromSpriteCoords(const IntVector2& spriteCoords) const;

	IntVector2	GetSpriteCoordsFromIndex(int idx) const;
	int			GetIndexFromSpriteCoords(int x, int y) const;
	int			GetIndexFromSpriteCoords(const IntVector2& spriteCoords) const;

	int			GetSpritesCount() const;

	Texture2D*	GetTexture() const;


public:
	Texture2D*	m_texture = nullptr;
	IntVector2	m_layout;
	float		m_unitUVWidth = 1.f;
	float		m_unitUVHeight = 1.f;
};