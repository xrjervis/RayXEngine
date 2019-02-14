#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class Viewport {
public:
	Viewport() {}
	~Viewport() {}

public:
	D3D11_VIEWPORT m_d3d11Viewport;
};