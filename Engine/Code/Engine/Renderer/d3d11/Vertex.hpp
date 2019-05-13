#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include <vector>

struct VertexPCU {
	VertexPCU(const Vector3& pos, const Vector4& color, const Vector2& texCoords)
		: m_position(pos)
		, m_color(color)
		, m_texCoords(texCoords) {}

	Vector3 m_position;
	Vector4	m_color;
	Vector2 m_texCoords;

	static std::vector<D3D11_INPUT_ELEMENT_DESC> s_layout;
};

struct VertexPCUTBN {
	VertexPCUTBN(const Vector3& pos, const Vector4& color, const Vector2& texCoords, const Vector3& normal, const Vector3& tangent, const Vector3& bitangent) 
		: m_position(pos)
		, m_color(color)
		, m_texCoords(texCoords)
		, m_normal(normal)
		, m_tangent(tangent)
		, m_bitangent(bitangent) {}

	Vector3 m_position;
	Vector4	m_color;
	Vector2 m_texCoords;
	Vector3 m_normal = Vector3::ZERO;
	Vector3 m_tangent;
	Vector3 m_bitangent;

	static std::vector<D3D11_INPUT_ELEMENT_DESC> s_layout;
};

constexpr int MAX_BONES_PER_VERTEX = 4;

struct VertexSkeletal {
	// Number of bones that influence this vertex
	int boneCount;

	// Which bones
	int boneIndex[MAX_BONES_PER_VERTEX];

	// Bone weights. These must sum to 1
	float boneWeight[MAX_BONES_PER_VERTEX];

};