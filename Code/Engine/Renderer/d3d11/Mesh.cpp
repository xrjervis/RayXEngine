#include "Engine/Renderer/d3d11/Mesh.hpp"
#include "Engine/Math/Matrix44.hpp"

Mesh::Mesh(ePrimitiveType primitiveType, bool useIndices)
	: m_primitiveType(primitiveType) 
	, m_useIndices(useIndices) {

}

Mesh::~Mesh() {
	m_vertexBuffer.reset();
	m_indexBuffer.reset();
	m_inputLayout.reset();
}

void Mesh::Reset(ePrimitiveType primitiveType, bool useIndices /*= true*/) {
	m_primitiveType = primitiveType;
	m_useIndices = useIndices;
	m_vertices.clear();
	m_triangles.clear();
	m_vertexBuffer.reset();
	m_indexBuffer.reset();
	m_inputLayout.reset();
	m_isFinalized = false;
}

void Mesh::AddLine(const Vector3& startPos, const Vector3& endPos, const Rgba& color /*= Rgba::WHITE*/) {
	GUARANTEE_OR_DIE(m_primitiveType == PRIMITIVE_TYPE_LINELIST || m_primitiveType == PRIMITIVE_TYPE_LINESTRIP, "Invalid primitive type");
	GUARANTEE_OR_DIE(m_useIndices == false, "Cannot draw lines with using indices");

	Vector3 newStart = (Vector4(startPos, 1.f) * Matrix44::GameToEngine).xyz();
	Vector3 newEnd = (Vector4(endPos, 1.f) * Matrix44::GameToEngine).xyz();

	m_vertices.emplace_back(newStart, color.GetAsFloats(), Vector2(0.f, 0.f));
	m_vertices.emplace_back(newEnd, color.GetAsFloats(), Vector2(0.f, 0.f));
}

void Mesh::AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const Rgba& color /*= Rgba::WHITE*/) {
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

void Mesh::AddQuad(const Vector2& position, const Vector2& pivot, float width, float height, const AABB2& uv /*= AABB2()*/, const Rgba& color /*= Rgba::WHITE*/) {
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

//				 back
//		    __________
//		   /|  top   /|
//		  /_|______ / |
//		 |  |      |  |
//	left |  |______|__|  right
//		 | / bottom| / 
//		 |/________|/
//		    front    
void Mesh::AddCube(const Vector3& center, const Vector3& size, const Rgba& color /*= Rgba::WHITE*/) {
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

void Mesh::AddPoint3D(const Vector3& center, const Rgba& color /*= Rgba::WHITE*/) {
	Vector4 c = color.GetAsFloats();
	Vector2 uv(0.f, 0.f);
	Vector3 pos = (Vector4(center, 1.f) * Matrix44::GameToEngine).xyz();
	m_vertices.emplace_back(pos, c, uv);
}

void Mesh::AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const Rgba& color /*= Rgba::WHITE*/) {
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

void Mesh::AddQuad3D(const Vector3& position, float width, float height, const Vector3& rightVector, const Vector3& upVector, const AABB2& uv /*= AABB2()*/, const Rgba& color /*= Rgba::WHITE*/) {
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

void Mesh::PushVertex(const VertexMaster& vert) {
	m_vertices.emplace_back(vert);
}

void Mesh::PushTriangle(u32 a, u32 b, u32 c) {
	m_triangles.push_back({a, b, c});
}
