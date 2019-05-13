#include "Engine/Renderer/d3d11/Sprite.hpp"

Sprite::Sprite(Texture2D* texture, const AABB2& uv /*= AABB2()*/) 
	: m_texture(texture)
	, m_uv(uv) {

}

Sprite::Sprite() {
}

void Sprite::SetTexture(Texture2D* texture) {
	m_texture = texture;
}

void Sprite::SetBillboard(bool useBillboard) {
	m_useBillboard = useBillboard;
}

void Sprite::SetSize(float width, float height) {
	m_width = width;
	m_height = height;
}

void Sprite::SetPivot(float pivotX, float pivotY) {
	m_pivot.x = pivotX;
	m_pivot.y = pivotY;
}

void Sprite::SetUV(const AABB2& uv) {
	m_uv = uv;
}

void Sprite::SetTint(const Rgba& tint) {
	m_tint = tint;
}
