#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class Text {
public:
	Text() = default;
	~Text() = default;

	ComPtr<IDWriteFactory>			m_DWriteFactory;
	ComPtr<IDWriteTextFormat>		m_textFormat;

	// direct2d
	ComPtr<ID2D1Factory>			m_D2DFactory;
	ComPtr<ID2D1RenderTarget>		m_renderTarget;
	ComPtr<ID2D1SolidColorBrush>	m_textBrush;
};