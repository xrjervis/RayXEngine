#include "Engine/Renderer/d3d11/SpriteSheet.hpp"

SpriteSheet::SpriteSheet(Texture2D* texture, const IntVector2& layout) 
	: m_texture(texture) 
	, m_layout(layout){
	m_unitUVWidth = 1.f / (float)layout.x;
	m_unitUVHeight = 1.f / (float)layout.y;
}