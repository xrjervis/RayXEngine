#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Renderer/d3d11/RHIDevice.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/Window.hpp"



RHIOutput::RHIOutput(RHIDevice* device, const std::wstring& windowTitle, float height, float aspect)
	: m_device(device) {
	// set back buffer size
	m_backBufferHeight = height;
	m_backBufferWidth = height * aspect;

	CreateOutputWindow(windowTitle, height, aspect);
	CreateSwapChain();
	CreateRenderTarget();
	CreateDepthStencilTarget();
}

RHIOutput::~RHIOutput() {

}

void RHIOutput::SwapBuffer() {
	m_swapChain->Present(0, 0);
}

void RHIOutput::CreateSwapChain() {
	// Create a descriptor for the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = (UINT)m_backBufferWidth;
	swapChainDesc.Height = (UINT)m_backBufferHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;

	// Create a SwapChain from a Win32 window.
	HR(m_device->m_rhi->m_dxgiFactory->CreateSwapChainForHwnd(
		m_device->m_d3d11Device.Get(),
		(HWND)m_window->GetHandle(),
		&swapChainDesc,
		&fsSwapChainDesc,
		nullptr, m_swapChain.ReleaseAndGetAddressOf()
	));

	// This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
	HR(m_device->m_rhi->m_dxgiFactory->MakeWindowAssociation((HWND)m_window->GetHandle(), DXGI_MWA_NO_ALT_ENTER));
}

void RHIOutput::CreateOutputWindow(const std::wstring& windowTitle, float height, float aspect) {
	m_window = std::make_unique<Window>(windowTitle, height, aspect);
}

void RHIOutput::CreateRenderTarget() {
	// Create a render target view of the swap chain back buffer.
	m_renderTargetTexture = m_device->CreateTexture2DFromSwapChain(this);
	m_renderTargetTexture->CreateRenderTargetView();
}

void RHIOutput::CreateDepthStencilTarget() {
	// Create a depth stencil view for use with 3D rendering if needed.
	TextureDefinition_t texDefn;
	texDefn.m_format = TEXTURE_FORMAT_D24_UNORM_S8_UINT;
	texDefn.m_dimension = Vector2(m_backBufferWidth, m_backBufferHeight);
	texDefn.m_bindFlags = BIND_DEPTH_STENCIL;
	texDefn.m_usage = MEMORY_USAGE_DEFAULT;
	texDefn.m_mipLevels = 1;
	texDefn.m_sampleCount = 1;
	texDefn.m_sampleQuality = 0;
	texDefn.m_initialData = nullptr;

	m_depthStencilTexture = m_device->CreateTexture2D(texDefn);
	m_depthStencilTexture->CreateDepthStencilView();
}