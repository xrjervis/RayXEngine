#include "Engine/Renderer/d3d11/ShaderProgram.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Logger.hpp"

ShaderProgram::ShaderProgram(const std::string& filePath, eShaderType shaderType)
	: m_srcFilePath(filePath)
	, m_type(shaderType) {
	Compile();
}

ShaderProgram::~ShaderProgram() {
	m_d3d11VertexShader.Reset();
	m_d3d11HullShader.Reset();
	m_d3d11DomainShader.Reset();
	m_d3d11GeometryShader.Reset();
	m_d3d11PixelShader.Reset();
	m_d3d11ComputeShader.Reset();
}

void ShaderProgram::Compile() {
	std::string entryPoint = ToDXShaderEntryPoint(m_type);
	std::string compileModel = ToDXShaderCompileModel(m_type);
	DWORD compileFlags = 0U;
#if defined _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
	compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
//	compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#else
	compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ComPtr<ID3DBlob> error;

	char* buffer = FileSystem::FileReadToBuffer(m_srcFilePath.c_str());
	size_t size = ::strlen(buffer);

	HRESULT hr = (::D3DCompile(buffer, size, m_srcFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
		entryPoint.c_str(), compileModel.c_str(), compileFlags, 0, m_compiledBlob.ReleaseAndGetAddressOf(), error.GetAddressOf()));


	if(error || hr != S_OK) {
		char* errorString = (char*)error->GetBufferPointer();
		ERROR_AND_DIE(Stringf("Failed to compile [%s].  Compiler gave the following output;\n%s",
			m_srcFilePath.c_str(),
			errorString).c_str());
	}

	delete[] buffer;
}

