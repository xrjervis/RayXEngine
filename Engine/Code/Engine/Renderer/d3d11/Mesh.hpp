#pragma once
#include "Engine/Renderer/d3d11/Vertex.hpp"
#include "Engine/Renderer/d3d11/Buffer.hpp"
#include "Engine/Renderer/d3d11/InputLayout.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include <array>

template<typename VertType>
class Mesh{
public:
	Mesh(ePrimitiveType primitiveType, bool useIndices = true);
	~Mesh();

	void Reset(ePrimitiveType primitiveType, bool useIndices = true);

	// AddPoint
	void AddLine(const Vector3& startPos, const Vector3& endPos, const Rgba& color = Rgba::WHITE);
	void AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const Rgba& color = Rgba::WHITE); // a 2d screen space quad, differed from plane
	void AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const AABB2& uv = AABB2(), const Rgba& color = Rgba::WHITE); // a 2d screen space quad, differed from plane
	void AddOBB2D(const Vector2& position, const Vector2& rightVector, const Vector2& upVector, float halfWidth, float halfHeight, const AABB2& uv = AABB2(), const Rgba& color = Rgba::WHITE);
	void AddDisc2D(const Vector2& center, float radius, const Rgba& color = Rgba::WHITE);
	void AddDashedCircle2D(const Vector2& center, float radius, const Rgba& color = Rgba::WHITE);

	void AddCube(const Vector3& center, const Vector3& size, const Rgba& color = Rgba::WHITE);
	void AddPoint3D(const Vector3& center, const Rgba& color = Rgba::WHITE);
	void AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const Rgba& color = Rgba::WHITE); // position is its the bottom left corner
	void AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const AABB2& uv = AABB2(), const Rgba& color = Rgba::WHITE); // position is its the bottom left corner
	void AddAABB3Box(const Vector3& center, const Vector3& size, const Rgba& color = Rgba::WHITE);
	void AddUVSphere(const Vector3& center, float radius, int longitude = 16, int latitude = 16, const Rgba& color = Rgba::WHITE);

	void PushVertex(const VertType& vert);
	void PushTriangle(u32 a, u32 b, u32 c);

	u32 GetVerticesCount() const { return (u32)m_vertices.size(); }
	u32 GetIndicesCount() const { return (u32)m_triangles.size() * 3U; }
	u32 GetTrianglesCount() const { return (u32)m_triangles.size(); }

public:
	bool							m_useIndices = true;
	bool							m_isFinalized = false;

	std::vector<VertType>			m_vertices;
	std::vector<std::array<u32, 3>>	m_triangles;
	ePrimitiveType					m_primitiveType;

	std::unique_ptr<Buffer>			m_vertexBuffer;
	std::unique_ptr<Buffer>			m_indexBuffer;
	std::unique_ptr<InputLayout>	m_inputLayout;
};



template<typename VertType>
Mesh<VertType>::Mesh(ePrimitiveType primitiveType, bool useIndices /*= true*/) 
	: m_primitiveType(primitiveType)
	, m_useIndices(useIndices) {}

template<typename VertType>
Mesh<VertType>::~Mesh() {
	m_vertexBuffer.reset();
	m_indexBuffer.reset();
	m_inputLayout.reset();
}

template<typename VertType>
void Mesh<VertType>::Reset(ePrimitiveType primitiveType, bool useIndices /*= true*/) {
	m_primitiveType = primitiveType;
	m_useIndices = useIndices;
	m_vertices.clear();
	m_triangles.clear();
	m_vertexBuffer.reset();
	m_indexBuffer.reset();
	m_inputLayout.reset();
	m_isFinalized = false;
}

template<typename VertType>
void Mesh<VertType>::AddLine(const Vector3& startPos, const Vector3& endPos, const Rgba& color /*= Rgba::WHITE*/) {
	GUARANTEE_OR_DIE(m_primitiveType == PRIMITIVE_TYPE_LINELIST || m_primitiveType == PRIMITIVE_TYPE_LINESTRIP, "Invalid primitive type");
	GUARANTEE_OR_DIE(m_useIndices == false, "Cannot draw lines with using indices");

	Vector3 newStart = (Vector4(startPos, 1.f) * Matrix44::GameToEngine).xyz();
	Vector3 newEnd = (Vector4(endPos, 1.f) * Matrix44::GameToEngine).xyz();

	m_vertices.emplace_back(newStart, color.GetAsFloats(), Vector2(0.f, 0.f));
	m_vertices.emplace_back(newEnd, color.GetAsFloats(), Vector2(0.f, 0.f));
}

template<typename VertType>
void Mesh<VertType>::AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const Rgba& color /*= Rgba::WHITE*/) {
	Vector3 tl(position.x - pivot.x * width, position.y + pivot.y * height, 0.f);
	Vector3 tr = tl + Vector3(width, 0.f, 0.f);
	Vector3 bl = tl + Vector3(0.f, -height, 0.f);
	Vector3 br = bl + Vector3(width, 0.f, 0.f);

	Vector4 c = color.GetAsFloats();

	Vector2 tl_uv(0.f, 0.f);
	Vector2 tr_uv(1.f, 0.f);
	Vector2 bl_uv(0.f, 1.f);
	Vector2 br_uv(1.f, 1.f);

	u32 vertCount = (u32)m_vertices.size();
	m_vertices.emplace_back(bl, c, bl_uv);
	m_vertices.emplace_back(br, c, br_uv);
	m_vertices.emplace_back(tl, c, tl_uv);
	m_vertices.emplace_back(tr, c, tr_uv);

	//ccw
	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });
}

template<typename VertType>
void Mesh<VertType>::AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const AABB2& uv /*= AABB2()*/, const Rgba& color /*= Rgba::WHITE*/) {
	Vector3 tl(position.x - pivot.x * width, position.y + pivot.y * height, 0.f);
	Vector3 tr = tl + Vector3(width, 0.f, 0.f);
	Vector3 bl = tl + Vector3(0.f, -height, 0.f);
	Vector3 br = bl + Vector3(width, 0.f, 0.f);

	Vector4 c = color.GetAsFloats();

	Vector2 tl_uv(uv.mins.x, uv.mins.y);
	Vector2 tr_uv(uv.maxs.x, uv.mins.y);
	Vector2 bl_uv(uv.mins.x, uv.maxs.y);
	Vector2 br_uv(uv.maxs.x, uv.maxs.y);

	u32 vertCount = (u32)m_vertices.size();
	m_vertices.emplace_back(bl, c, bl_uv);
	m_vertices.emplace_back(br, c, br_uv);
	m_vertices.emplace_back(tl, c, tl_uv);
	m_vertices.emplace_back(tr, c, tr_uv);

	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });
}

template<typename VertType>
void Mesh<VertType>::AddOBB2D(const Vector2& position, const Vector2& rightVector, const Vector2& upVector, float halfWidth, float halfHeight, const AABB2& uv /*= AABB2()*/, const Rgba& color /*= Rgba::WHITE*/) {
	Vector3 tr = position + rightVector * halfWidth + upVector * halfHeight;
	Vector3 tl = tr + rightVector * (-halfWidth * 2.f);
	Vector3 bl = tl + upVector * (-halfHeight * 2.f);
	Vector3 br = bl + rightVector * (halfWidth * 2.f);

	Vector4 c = color.GetAsFloats();

	Vector2 tl_uv(uv.mins.x, uv.mins.y);
	Vector2 tr_uv(uv.maxs.x, uv.mins.y);
	Vector2 bl_uv(uv.mins.x, uv.maxs.y);
	Vector2 br_uv(uv.maxs.x, uv.maxs.y);

	u32 vertCount = (u32)m_vertices.size();
	m_vertices.emplace_back(bl, c, bl_uv);
	m_vertices.emplace_back(br, c, br_uv);
	m_vertices.emplace_back(tl, c, tl_uv);
	m_vertices.emplace_back(tr, c, tr_uv);

	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });
}

template<typename VertType>
void Mesh<VertType>::AddDisc2D(const Vector2& center, float radius, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();

	for (int i = 0; i < 36; ++i) {
		float curDegrees = (float)i * 10.f;
		float nextDegrees = (float)(i + 1) * 10.f;

		Vector2 curDir = PolarToCartesian(radius, curDegrees).GetNormalized();
		Vector2 curVertexPos = center + radius * curDir;

		Vector2 nextDir = PolarToCartesian(radius, nextDegrees).GetNormalized();
		Vector2 nextVertexPos = center + radius * nextDir;

		m_vertices.emplace_back(center, c, Vector2::ZERO);
		m_vertices.emplace_back(curVertexPos, c, Vector2::ZERO);
		m_vertices.emplace_back(nextVertexPos, c, Vector2::ZERO);
	}
}


template<typename VertType>
void Mesh<VertType>::AddDashedCircle2D(const Vector2& center, float radius, const Rgba& color /*= Rgba::WHITE*/) {
	for (int i = 0; i < 36; ++i) {
		if (i % 2 != 0) {
			continue;
		}
		float curDegrees = (float)i * 10.f;
		float nextDegrees = (float)(i + 1) * 10.f;

		Vector2 curDir = PolarToCartesian(radius, curDegrees).GetNormalized();
		Vector2 curVertexPos = center + radius * curDir;

		Vector2 nextDir = PolarToCartesian(radius, nextDegrees).GetNormalized();
		Vector2 nextVertexPos = center + radius * nextDir;

		AddLine(curVertexPos, nextVertexPos, color);
	}
}

//				 back
//		    __________
//		   /|  top   /|
//		  /_|______ / |
//		 |  |      |  |
//	left |  |______|__|  right
//		 | / bottom| / 
//		 |/________|/
//		    front    
template<typename VertType>
void Mesh<VertType>::AddCube(const Vector3& center, const Vector3& size, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();

	Vector3 newCenter = (Vector4(center, 1.f) * Matrix44::GameToEngine).xyz();

	//front vertices
	Vector3 blfCorner = newCenter - (size * 0.5f);
	Vector3 brfCorner = blfCorner + size.x * Vector3(1.f, 0.f, 0.f);
	Vector3 tlfCorner = blfCorner + size.y * Vector3(0.f, 1.f, 0.f);
	Vector3 trfCorner = tlfCorner + size.x * Vector3(1.f, 0.f, 0.f);

	//back vertices
	Vector3 trbCorner = newCenter + (size * 0.5f);
	Vector3 tlbCorner = trbCorner - size.x * Vector3(1.f, 0.f, 0.f);
	Vector3 blbCorner = tlbCorner - size.y * Vector3(0.f, 1.f, 0.f);
	Vector3 brbCorner = blbCorner + size.x * Vector3(1.f, 0.f, 0.f);

	Vector2 tl_uv(0.f, 0.f);
	Vector2 tr_uv(1.f, 0.f);
	Vector2 bl_uv(0.f, 1.f);
	Vector2 br_uv(1.f, 1.f);

	u32 vertCount = (u32)m_vertices.size();
	// front face 0,1,2,3
	m_vertices.emplace_back(blfCorner, c, bl_uv);
	m_vertices.emplace_back(brfCorner, c, br_uv);
	m_vertices.emplace_back(tlfCorner, c, tl_uv);
	m_vertices.emplace_back(trfCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });

	// back face 4,5,6,7
	m_vertices.emplace_back(brbCorner, c, bl_uv);
	m_vertices.emplace_back(blbCorner, c, br_uv);
	m_vertices.emplace_back(trbCorner, c, tl_uv);
	m_vertices.emplace_back(tlbCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 4, vertCount + 5, vertCount + 6 });
	m_triangles.push_back({ vertCount + 5, vertCount + 7, vertCount + 6 });

	// left face 8,9,10,11
	m_vertices.emplace_back(blbCorner, c, bl_uv);
	m_vertices.emplace_back(blfCorner, c, br_uv);
	m_vertices.emplace_back(tlbCorner, c, tl_uv);
	m_vertices.emplace_back(tlfCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 8, vertCount + 9,  vertCount + 10 });
	m_triangles.push_back({ vertCount + 9, vertCount + 11, vertCount + 10 });

	// right face 12,13,14,15
	m_vertices.emplace_back(brfCorner, c, bl_uv);
	m_vertices.emplace_back(brbCorner, c, br_uv);
	m_vertices.emplace_back(trfCorner, c, tl_uv);
	m_vertices.emplace_back(trbCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 12, vertCount + 13, vertCount + 14 });
	m_triangles.push_back({ vertCount + 13, vertCount + 15, vertCount + 14 });

	// top face 16,17,18,19
	m_vertices.emplace_back(tlfCorner, c, bl_uv);
	m_vertices.emplace_back(trfCorner, c, br_uv);
	m_vertices.emplace_back(tlbCorner, c, tl_uv);
	m_vertices.emplace_back(trbCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 16, vertCount + 17, vertCount + 18 });
	m_triangles.push_back({ vertCount + 17, vertCount + 19, vertCount + 18 });

	// bottom face 20,21,22,23
	m_vertices.emplace_back(blbCorner, c, bl_uv);
	m_vertices.emplace_back(brbCorner, c, br_uv);
	m_vertices.emplace_back(blfCorner, c, tl_uv);
	m_vertices.emplace_back(brfCorner, c, tr_uv);

	m_triangles.push_back({ vertCount + 20, vertCount + 21, vertCount + 22 });
	m_triangles.push_back({ vertCount + 21, vertCount + 23, vertCount + 22 });
}

template<typename VertType>
void Mesh<VertType>::AddPoint3D(const Vector3& center, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();
	Vector2 uv(0.f, 0.f);
	Vector3 pos = (Vector4(center, 1.f) * Matrix44::GameToEngine).xyz();
	m_vertices.emplace_back(pos, c, uv);
}

template<typename VertType>
void Mesh<VertType>::AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();
	Vector3 newPos = (Vector4(position, 1.f) * Matrix44::GameToEngine).xyz();
	Vector3 newRight = (Vector4(rightVector, 0.f) * Matrix44::GameToEngine).xyz();
	Vector3 newUp = (Vector4(upVector, 0.f) * Matrix44::GameToEngine).xyz();

	Vector3 bl = newPos;
	Vector3 br = bl + width * newRight;
	Vector3 tl = bl + height * newUp;
	Vector3 tr = tl + width * newRight;

	Vector2 tl_uv(0.f, 0.f);
	Vector2 tr_uv(1.f, 0.f);
	Vector2 bl_uv(0.f, 1.f);
	Vector2 br_uv(1.f, 1.f);

	u32 vertCount = (u32)m_vertices.size();
	m_vertices.emplace_back(bl, c, bl_uv);
	m_vertices.emplace_back(br, c, br_uv);
	m_vertices.emplace_back(tl, c, tl_uv);
	m_vertices.emplace_back(tr, c, tr_uv);

	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });
}

template<typename VertType>
void Mesh<VertType>::AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const AABB2& uv /*= AABB2()*/, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();
	Vector3 newPos = (Vector4(position, 1.f) * Matrix44::GameToEngine).xyz();
	Vector3 newRight = (Vector4(rightVector, 0.f) * Matrix44::GameToEngine).xyz();
	Vector3 newUp = (Vector4(upVector, 0.f) * Matrix44::GameToEngine).xyz();

	Vector3 bl = newPos;
	Vector3 br = bl + width * newRight;
	Vector3 tl = bl + height * newUp;
	Vector3 tr = tl + width * newRight;

	Vector2 tl_uv(uv.mins.x, uv.mins.y);
	Vector2 tr_uv(uv.maxs.x, uv.mins.y);
	Vector2 bl_uv(uv.mins.x, uv.maxs.y);
	Vector2 br_uv(uv.maxs.x, uv.maxs.y);

	u32 vertCount = (u32)m_vertices.size();
	m_vertices.emplace_back(bl, c, bl_uv);
	m_vertices.emplace_back(br, c, br_uv);
	m_vertices.emplace_back(tl, c, tl_uv);
	m_vertices.emplace_back(tr, c, tr_uv);

	m_triangles.push_back({ vertCount + 0, vertCount + 1, vertCount + 2 });
	m_triangles.push_back({ vertCount + 1, vertCount + 3, vertCount + 2 });
}


template<typename VertType>
void Mesh<VertType>::AddAABB3Box(const Vector3& center, const Vector3& size, const Rgba& color /*= Rgba::WHITE*/) {
	Vector3 blf = center - 0.5f * size;
	Vector3 brf = blf + Vector3(size.x, 0.f, 0.f);
	Vector3 tlf = blf + Vector3(0.f, 0.f, size.z);
	Vector3 trf = tlf + Vector3(size.x, 0.f, 0.f);
	Vector3 blb = blf + Vector3(0.f, size.y, 0.f);
	Vector3 brb = blb + Vector3(size.x, 0.f, 0.f);
	Vector3 tlb = blb + Vector3(0.f, 0.f, size.z);
	Vector3 trb = tlb + Vector3(size.x, 0.f, 0.f);

	AddLine(blf, brf, color);
	AddLine(brf, trf, color);
	AddLine(trf, tlf, color);
	AddLine(tlf, blf, color);

	AddLine(brf, brb, color);
	AddLine(brb, trb, color);
	AddLine(trb, trf, color);
	AddLine(trf, brf, color);

	AddLine(brb, blb, color);
	AddLine(blb, tlb, color);
	AddLine(tlb, trb, color);
	AddLine(trb, brb, color);

	AddLine(blb, blf, color);
	AddLine(blf, tlf, color);
	AddLine(tlf, tlb, color);
	AddLine(tlb, blb, color);
}


template<typename VertType>
void Mesh<VertType>::AddUVSphere(const Vector3& center, float radius, int longitude /*= 16*/, int latitude /*= 16*/, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();
	Vector3 newCenter = (Vector4(center, 1.f) * Matrix44::GameToEngine).xyz();
	u32 vertCount = (u32)m_vertices.size();

	for (int vIdx = 0; vIdx <= latitude; ++vIdx) {
		float v = (float)vIdx / (float)latitude;
		float rotation = 180.f * v;

		for (int uIdx = 0; uIdx < longitude; ++uIdx) {
			float u = (float)uIdx / (float)longitude;
			float azimuth = 360.f * u;

			Vector3 pos = newCenter + PolarToCartesian(radius, rotation, azimuth);
			m_vertices.emplace_back(pos, c, Vector2(u, v));
		}
	}

	for (u32 vIdx = 0; vIdx < latitude; ++vIdx) {
		for (u32 uIdx = 0; uIdx < longitude; ++uIdx) {
			u32 tlIdx = vIdx * longitude + uIdx + vertCount;
			u32 trIdx = vIdx * longitude + ((uIdx + 1)%longitude) + vertCount;
			u32 blIdx = (vIdx + 1) * longitude + uIdx + vertCount;
			u32 brIdx = (vIdx + 1) * longitude + ((uIdx + 1) % longitude) + vertCount;

			m_triangles.push_back({ blIdx, trIdx, tlIdx });
			m_triangles.push_back({ blIdx, brIdx, trIdx});
		}
	}
}



template<typename VertType>
void Mesh<VertType>::PushVertex(const VertType& vert) {
	m_vertices.emplace_back(vert);
}

template<typename VertType>
void Mesh<VertType>::PushTriangle(u32 a, u32 b, u32 c) {
	m_triangles.push_back({ a, b, c });
}