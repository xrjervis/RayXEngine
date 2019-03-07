#pragma once
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"
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

inline AABB2 SpriteSheet::GetUVFromIndex(int idx) const {
	idx = ClampInt(idx, 0, GetSpritesCount() - 1);
	IntVector2 spriteCoords = GetSpriteCoordsFromIndex(idx);
	return GetUVFromSpriteCoords(spriteCoords);
}

inline AABB2 SpriteSheet::GetUVFromSpriteCoords(int x, int y) const {
	Vector2 t_min((float)x * m_unitUVWidth, (float)y * m_unitUVHeight);
	Vector2 t_max(t_min + Vector2(m_unitUVWidth, m_unitUVHeight));
	return AABB2(t_min, t_max);
}

inline AABB2 SpriteSheet::GetUVFromSpriteCoords(const IntVector2& spriteCoords) const {
	return GetUVFromSpriteCoords(spriteCoords.x, spriteCoords.y);
}

inline IntVector2 SpriteSheet::GetSpriteCoordsFromIndex(int idx) const {
	int row = idx / m_layout.y;
	int column = idx % m_layout.x;
	return IntVector2(column, row);
}

inline int SpriteSheet::GetIndexFromSpriteCoords(int x, int y) const {
	return y * m_layout.x + x;
}

inline int SpriteSheet::GetIndexFromSpriteCoords(const IntVector2& spriteCoords) const {
	return GetIndexFromSpriteCoords(spriteCoords.x, spriteCoords.y);
}

inline int SpriteSheet::GetSpritesCount() const {
	return m_layout.x * m_layout.y;
}

inline Texture2D* SpriteSheet::GetTexture() const {
	return m_texture;
}
