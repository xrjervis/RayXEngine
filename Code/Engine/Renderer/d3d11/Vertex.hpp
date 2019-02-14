#pragma once
#include "Engine/Renderer/d3d11/D3DCommon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include <vector>

struct VertexMaster {
	VertexMaster(const Vector3& pos, const Vector4& color, const Vector2& texCoords)
		: m_position(pos)
		, m_color(color)
		, m_texCoords(texCoords) {}

	VertexMaster(const Vector3& pos, const Vector4& color, const Vector2& texCoords, const Vector3& normal)
		: m_position(pos)
		, m_color(color)
		, m_texCoords(texCoords)
		, m_normal(normal) {}

	VertexMaster(const Vector3& pos, const Vector4& color, const Vector2& texCoords, const Vector3& normal, const Vector3& tangent, const Vector3& bitangent)
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

// 	Vector4 m_boneWeights;
// 	Vector4 m_boneIndices;

	static std::vector<D3D11_INPUT_ELEMENT_DESC> s_layoutDescs;
};