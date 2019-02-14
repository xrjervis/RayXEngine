#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"

class InputLayout{
public:
	InputLayout() = default;
	~InputLayout() = default;

public:
	ComPtr<ID3D11InputLayout> m_d3d11InputLayout;
};