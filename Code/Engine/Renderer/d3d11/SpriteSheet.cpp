#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteSheet::SpriteSheet(Texture2D* texture, const IntVector2& layout) 
	: m_texture(texture) 
	, m_layout(layout){
	m_unitUVWidth = 1.f / (float)layout.x;
	m_unitUVHeight = 1.f / (float)layout.y;
}

AABB2 SpriteSheet::GetUVFromIndex(int idx) const {
	idx = ClampInt(idx, 0, GetSpritesCount() - 1);
	IntVector2 spriteCoords = GetSpriteCoordsFromIndex(idx);
	return GetUVFromSpriteCoords(spriteCoords);
}

AABB2 SpriteSheet::GetUVFromSpriteCoords(int x, int y) const {
	Vector2 min((float)x * m_unitUVWidth, (float)y * m_unitUVHeight);
	Vector2 max(min + Vector2(m_unitUVWidth, m_unitUVHeight));
	return AABB2(min, max);
}

AABB2 SpriteSheet::GetUVFromSpriteCoords(const IntVector2& spriteCoords) const {
	return GetUVFromSpriteCoords(spriteCoords.x, spriteCoords.y);
}

IntVector2 SpriteSheet::GetSpriteCoordsFromIndex(int idx) const {
	int row = idx / m_layout.y;
	int column = idx % m_layout.x;
	return IntVector2(column, row);
}

int SpriteSheet::GetIndexFromSpriteCoords(int x, int y) const {
	return y * m_layout.x + x;
}

int SpriteSheet::GetIndexFromSpriteCoords(const IntVector2& spriteCoords) const {
	return GetIndexFromSpriteCoords(spriteCoords.x, spriteCoords.y);
}

int SpriteSheet::GetSpritesCount() const {
	return m_layout.x * m_layout.y;
}

Texture2D* SpriteSheet::GetTexture() const {
	return m_texture;
}
