#pragma once
#include "Engine/Renderer/d3d11/Text.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include <memory>

enum eTextAlignment {
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT,
	TEXT_ALIGN_TOP,
	TEXT_ALIGN_BOTTOM
};

class RHIOutput;

class FontRenderer {
public:
	FontRenderer();
	~FontRenderer();

	void SetFont(const std::wstring& fontName);
	void SetSize(float size);
	void SetAlignment(eTextAlignment horizontal, eTextAlignment vertical = TEXT_ALIGN_TOP);

	// will update device dependent resources
	void BindOutput(RHIOutput* output);
	void SetColor(const Rgba& color);

	void DrawText(const std::string& str, const Rgba& color = Rgba::WHITE);
	void DrawTextInBox(const std::string& str, const AABB2& box, const Rgba& color = Rgba::WHITE);

	float GetTextWidth(const std::string& str, const AABB2& box);

private:
	// create device independent resources
	void InitDirectWrite();
	void UpdateRenderTarget();
	void UpdateTextFormat();
	void UpdateTextBrush();

private:
	std::wstring			m_fontName = L"Verdana";
	float					m_fontSize = 72.f;
	Rgba					m_fontColor = Rgba::WHITE;
	std::wstring			m_textString;

	RHIOutput*				m_output = nullptr;
	std::unique_ptr<Text>	m_d2dTextObj;
};