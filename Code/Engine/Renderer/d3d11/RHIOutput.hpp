#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include "Engine/Renderer/d3d11/ResourceView.hpp"
#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Core/Window.hpp"
#include <memory>

class RHIDevice;

class RHIOutput{
public:
	RHIOutput(RHIDevice* device, const std::wstring& windowTitle, float height, float aspect);
	~RHIOutput();

	Window*							GetWindow() const { return m_window.get(); }
	RenderTargetView*				GetRTV() const { return m_renderTargetTexture->m_rtv.get(); }
	DepthStencilView*				GetDSV() const { return m_depthStencilTexture->m_dsv.get(); }
	float							GetWidth() const { return m_backBufferWidth; }
	float							GetHeight() const { return m_backBufferHeight; }
	Vector2							GetCenter() const { return Vector2(m_backBufferWidth * 0.5f, m_backBufferHeight * 0.5f); }
	Vector2							GetDimension() const { return Vector2(m_backBufferWidth, m_backBufferHeight); }
	float							GetAspect() const { return m_backBufferWidth / m_backBufferHeight; }
	void							SwapBuffer();

private:
	void							CreateSwapChain();
	void							CreateOutputWindow(const std::wstring& windowTitle, float height, float aspect);
	void							CreateRenderTarget();
	void							CreateDepthStencilTarget();

public:
	float							m_backBufferWidth;
	float							m_backBufferHeight;

	ComPtr<IDXGISwapChain1>			m_swapChain;
	std::unique_ptr<Window>			m_window;
	RHIDevice*						m_device = nullptr;
	std::unique_ptr<Texture2D>		m_renderTargetTexture;
	std::unique_ptr<Texture2D>      m_depthStencilTexture;
};