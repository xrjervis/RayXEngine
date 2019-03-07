#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class RenderState {
public:
	RenderState() {}
	~RenderState() {
		m_d3d11RasterizerState.Reset();
		m_d3d11BlendState.Reset();
		m_d3d11DepthStencilState.Reset();
	}

public:
	union {
		ComPtr<ID3D11RasterizerState>	m_d3d11RasterizerState;
		ComPtr<ID3D11BlendState>		m_d3d11BlendState;
		ComPtr<ID3D11DepthStencilState> m_d3d11DepthStencilState;
	};
};