#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class Sampler{
public:
	Sampler() {}
	~Sampler() {}

public:
	ComPtr<ID3D11SamplerState> m_d3d11Sampler;
};