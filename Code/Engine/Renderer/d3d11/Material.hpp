#pragma once
#include "Engine/Renderer/d3d11/RenderState.hpp" 
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include <memory>

class Texture2D;
class ShaderProgram;

// in hlsl bool is 4 bytes; but in cpp bool is 1 byte
struct MaterialBuffer_t{
// 	Vector4 emissive = Vector4(0, 0, 0, 0);  //16
// 	Vector4 diffuse = Vector4(0, 0, 0, 0);   //32
// 	Vector4 specular = Vector4(0, 0, 0, 0);  //48
// 	float specularPower = 32.f;              //52
// 	int useDiffuse = false;					 //56
// 	int useNormal = false;					 //60
// 	int padding1;							 //64
// 	
	int useDiffuse = 0;
	int useNormal = 0;
	int useSpecular = 0;
	int useLighting = 0;
};

class Material{
public:
	Material();
	~Material();

public:
	Texture2D*						m_diffuse = nullptr;
	Texture2D*						m_normal = nullptr;
	
	ShaderProgram*					m_vs = nullptr;
	ShaderProgram*					m_ps = nullptr;

	std::unique_ptr<RenderState>	m_rasterizerState;
	std::unique_ptr<RenderState>	m_depthStencilState;
	std::unique_ptr<RenderState>	m_blendState;

	MaterialBuffer_t				m_data;
};