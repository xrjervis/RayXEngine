#pragma once
#include "Engine/Renderer/d3d11/ShaderProgram.hpp"

class Shader {
public:
	Shader();
	~Shader();

public:
	std::unique_ptr<ShaderProgram> m_shaderProgram;
};