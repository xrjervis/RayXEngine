#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class RenderTargetView {
public:
	RenderTargetView() {}
	~RenderTargetView() {}

public:
	ComPtr<ID3D11RenderTargetView>	m_d3d11RTV;
};

class ShaderResourceView {
public:
	ShaderResourceView() {}
	~ShaderResourceView() {}

public:
	ComPtr<ID3D11ShaderResourceView> m_d3d11SRV;
};

class DepthStencilView {
public:
	DepthStencilView() {}
	~DepthStencilView() {}
public:
	ComPtr<ID3D11DepthStencilView> m_d3d11DSV;
};