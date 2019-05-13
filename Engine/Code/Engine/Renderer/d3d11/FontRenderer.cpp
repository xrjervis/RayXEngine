#include "Engine/Renderer/d3d11/FontRenderer.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

FontRenderer::FontRenderer() {
	m_d2dTextObj = std::make_unique<Text>();
	InitDirectWrite();
	UpdateTextFormat();
	SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_TOP);
}

FontRenderer::~FontRenderer() {

}

void FontRenderer::BindOutput(RHIOutput* output) {
	m_output = output;
	UpdateRenderTarget();
	UpdateTextBrush();
}

void FontRenderer::SetColor(const Rgba& color) {
	m_fontColor = color;
	UpdateTextBrush();
}

void FontRenderer::DrawText(const std::string& str, const Rgba& color) {
	SetColor(color);
	m_textString = std::wstring(str.begin(), str.end());
	// Retrieve the size of the render target.
	D2D1_SIZE_F renderTargetSize = m_d2dTextObj->m_renderTarget->GetSize();

	m_d2dTextObj->m_renderTarget->BeginDraw();
	m_d2dTextObj->m_renderTarget->DrawText(
		m_textString.c_str(), 
		(UINT)m_textString.length(), 
		m_d2dTextObj->m_textFormat.Get(), 
		D2D1::RectF(0, renderTargetSize.height, renderTargetSize.width, 0),
		m_d2dTextObj->m_textBrush.Get()
	);
	m_d2dTextObj->m_renderTarget->EndDraw();
}

void FontRenderer::DrawTextInBox(const std::string& str, const AABB2& box, const Rgba& color) {
	GUARANTEE_OR_DIE(m_d2dTextObj->m_renderTarget != nullptr, "Failed to set RenderTarget for FontRenderer");
	SetColor(color);
	m_textString = std::wstring(str.begin(), str.end());
	// Retrieve the size of the render target.
	D2D1_SIZE_F renderTargetSize = m_d2dTextObj->m_renderTarget->GetSize();

	FLOAT startX = box.mins.x;
	FLOAT startY = box.mins.y;
	FLOAT endX = box.maxs.x;
	FLOAT endY = box.maxs.y;

	m_d2dTextObj->m_renderTarget->BeginDraw();
	m_d2dTextObj->m_renderTarget->DrawText(
		m_textString.c_str(),
		(UINT)m_textString.length(),
		m_d2dTextObj->m_textFormat.Get(),
		D2D1::RectF(startX, renderTargetSize.height - startY, endX, renderTargetSize.height - endY),
		m_d2dTextObj->m_textBrush.Get()
	);
	m_d2dTextObj->m_renderTarget->EndDraw();
}

float FontRenderer::GetTextWidth(const std::string& str, const AABB2& box) {
	std::wstring wText(str.begin(), str.end());
	FLOAT startX = box.mins.x;
	FLOAT startY = box.mins.y;
	FLOAT endX = box.maxs.x;
	FLOAT endY = box.maxs.y;

	ComPtr<IDWriteTextLayout> textLayout;
	HR(m_d2dTextObj->m_DWriteFactory->CreateTextLayout(
		wText.c_str(),							// The string to be laid out and formatted.
		(UINT32)wText.length(),					// The length of the string.
		m_d2dTextObj->m_textFormat.Get(),		// The text format to apply to the string (contains font information, etc).
		endX - startX,							// The width of the layout box.
		endY - startY,							// The height of the layout box.
		textLayout.ReleaseAndGetAddressOf()		// The IDWriteTextLayout interface pointer.
	));
	DWRITE_TEXT_METRICS metrics;
	textLayout->GetMetrics(&metrics);
	return metrics.widthIncludingTrailingWhitespace;
}

void FontRenderer::InitDirectWrite() {
	HRESULT hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dTextObj->m_D2DFactory.GetAddressOf());
	if(SUCCEEDED(hr)){
		hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, 
			__uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(m_d2dTextObj->m_DWriteFactory.GetAddressOf()));
		if(FAILED(hr)){
			ERROR_AND_DIE("Create dwrite factory failed");
		}
	}
	else {
		ERROR_AND_DIE("Create d2d1 factory failed.");
	}
}

void FontRenderer::SetFont(const std::wstring& fontName) {
	m_fontName = fontName;
	UpdateTextFormat();
}

void FontRenderer::SetSize(float size) {
	m_fontSize = size;
	UpdateTextFormat();
}

void FontRenderer::SetAlignment(eTextAlignment horizontal, eTextAlignment vertical /*= TEXT_ALIGN_TOP*/) {
	DWRITE_TEXT_ALIGNMENT textAlign;
	switch(horizontal){
	case TEXT_ALIGN_LEFT: textAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
	case TEXT_ALIGN_CENTER: textAlign = DWRITE_TEXT_ALIGNMENT_CENTER; break;
	case TEXT_ALIGN_RIGHT: textAlign = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
	default: textAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
	}
	m_d2dTextObj->m_textFormat->SetTextAlignment(textAlign);

	DWRITE_PARAGRAPH_ALIGNMENT paragraphAlign;
	switch(vertical){
	case TEXT_ALIGN_TOP: paragraphAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR; break;
	case TEXT_ALIGN_CENTER: paragraphAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER; break;
	case TEXT_ALIGN_BOTTOM: paragraphAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR; break;
	default: paragraphAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR; break;
	}
	m_d2dTextObj->m_textFormat->SetParagraphAlignment(paragraphAlign);
}

void FontRenderer::UpdateRenderTarget() {
	if(m_output == nullptr){
		ERROR_AND_DIE("RHIOutput in FontRenderer is nullptr!");
	}
	ComPtr<IDXGISurface> surface;
	HR(m_output->m_swapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HR(m_d2dTextObj->m_D2DFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_d2dTextObj->m_renderTarget.ReleaseAndGetAddressOf()));
	surface.Reset();
}

void FontRenderer::UpdateTextFormat() {
	HR(m_d2dTextObj->m_DWriteFactory->CreateTextFormat(m_fontName.c_str(), NULL,
		DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		m_fontSize, L"en-us", m_d2dTextObj->m_textFormat.ReleaseAndGetAddressOf()));

}

void FontRenderer::UpdateTextBrush() {
	HR(m_d2dTextObj->m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(m_fontColor.GetFloatR(), m_fontColor.GetFloatG(), m_fontColor.GetFloatB(), m_fontColor.GetFloatA()),
		m_d2dTextObj->m_textBrush.ReleaseAndGetAddressOf()));
}

