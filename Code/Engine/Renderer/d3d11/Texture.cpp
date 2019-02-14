#include "Engine/Renderer/d3d11/Texture.hpp"
#include "Engine/Renderer/d3d11/RHIDevice.hpp"

Texture2D::Texture2D(RHIDevice* device)
	:RHIResource(device) {

}

Texture2D::~Texture2D() {

}

RenderTargetView* Texture2D::CreateRenderTargetView() {
	m_rtv = std::make_unique<RenderTargetView>();
	D3D11_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Format = ToDXGIFormat(m_definition.m_format);
	desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HR(GetDevice()->m_d3d11Device->CreateRenderTargetView(
		m_d3d11Texture2D.Get(),
		&desc,
		m_rtv->m_d3d11RTV.ReleaseAndGetAddressOf()
	));

	return m_rtv.get();
}

DepthStencilView* Texture2D::CreateDepthStencilView() {
	m_dsv = std::make_unique<DepthStencilView>();
	D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;

	HR(GetDevice()->m_d3d11Device->CreateDepthStencilView(
		m_d3d11Texture2D.Get(),
		&desc,
		m_dsv->m_d3d11DSV.ReleaseAndGetAddressOf()
	));

	return m_dsv.get();
}

ShaderResourceView* Texture2D::CreateShaderResourceView() {
	m_srv = std::make_unique<ShaderResourceView>();
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Format = ToDXGIFormat(m_definition.m_format);
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = m_definition.m_mipLevels;

	HR(GetDevice()->m_d3d11Device->CreateShaderResourceView(
		m_d3d11Texture2D.Get(),
		&desc,
		m_srv->m_d3d11SRV.ReleaseAndGetAddressOf()
	));

	return m_srv.get();
}
