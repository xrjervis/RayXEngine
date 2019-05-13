#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include <string>

class ShaderProgram {
public:
	ShaderProgram(const std::string& filePath, eShaderType shaderType);
	~ShaderProgram();

private:
	void Compile();

public:
	union{
		ComPtr<ID3D11VertexShader>		m_d3d11VertexShader;
		ComPtr<ID3D11HullShader>		m_d3d11HullShader;
		ComPtr<ID3D11DomainShader>		m_d3d11DomainShader;
		ComPtr<ID3D11GeometryShader>	m_d3d11GeometryShader;
		ComPtr<ID3D11PixelShader>		m_d3d11PixelShader;
		ComPtr<ID3D11ComputeShader>		m_d3d11ComputeShader;
	};

	std::string			m_srcFilePath;
	eShaderType			m_type;
	ComPtr<ID3DBlob>	m_compiledBlob;
};

