#pragma once
#include "Engine/Renderer/d3d11/Vertex.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Renderer/d3d11/InputLayout.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include <array>

class Mesh{
public:
	Mesh(ePrimitiveType primitiveType, bool useIndices = true);
	~Mesh();

	void Reset(ePrimitiveType primitiveType, bool useIndices = true);

	// AddPoint
	void AddLine(const Vector3& startPos, const Vector3& endPos, const Rgba& color = Rgba::WHITE);
	void AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const Rgba& color = Rgba::WHITE); // a 2d screen space quad, differed from plane
	void AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const AABB2& uv = AABB2(), const Rgba& color = Rgba::WHITE); // a 2d screen space quad, differed from plane

	void AddCube(const Vector3& center, const Vector3& size, const Rgba& color = Rgba::WHITE);
	void AddPoint3D(const Vector3& center, const Rgba& color = Rgba::WHITE);
	void AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const Rgba& color = Rgba::WHITE); // position is its the bottom left corner
	void AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const AABB2& uv = AABB2(), const Rgba& color = Rgba::WHITE); // position is its the bottom left corner

	void PushVertex(const VertexMaster& vert);
	void PushTriangle(u32 a, u32 b, u32 c);

	u32 GetVerticesCount() const { return (u32)m_vertices.size(); }
	u32 GetIndicesCount() const { return (u32)m_triangles.size() * 3U; }
	u32 GetTrianglesCount() const { return (u32)m_triangles.size(); }

public:
	bool							m_useIndices = true;
	bool							m_isFinalized = false;

	std::vector<VertexMaster>		m_vertices;
	std::vector<std::array<u32, 3>>	m_triangles;
	ePrimitiveType					m_primitiveType;

	std::unique_ptr<Buffer>			m_vertexBuffer;
	std::unique_ptr<Buffer>			m_indexBuffer;
	std::unique_ptr<InputLayout>	m_inputLayout;
};
