#include "Engine/Terrain/Terrain.hpp"
#include "Engine/Math/MathUtils.hpp"

Terrain::Terrain() {
	m_mesh = std::make_unique<Mesh>(PRIMITIVE_TYPE_TRIANGLELIST, true);
}

Terrain::~Terrain() {

}

// 256x256 Terrain needs a 257x257 height map
void Terrain::GenerateFromHeightMap(u32 gridWidth, u32 gridHeight, float scale, const Image& heightMap, const Image& colorMap) {
	m_heightValues.clear();
	m_normals.clear();
	m_tangents.clear();
	m_bitangents.clear();
	m_normalizedColorValues.clear();

	size_t heightValueSize = (gridWidth + 1) * (gridHeight + 1);
	m_heightValues.resize(heightValueSize);
	m_normals.resize(heightValueSize);
	m_tangents.resize(heightValueSize);
	m_bitangents.resize(heightValueSize);
	m_normalizedColorValues.resize(heightValueSize);

	// First, calculate height values
	for(u32 h = 0; h <= gridHeight; ++h){
		for(u32 w = 0; w <= gridWidth; ++w){
			u32 index = h * gridWidth + w;
			float normalizedHeightValue = (float)heightMap.GetTexel(w, h).r / 255.f;
			float scaledHeightValue = normalizedHeightValue * scale;
			m_heightValues[index] = scaledHeightValue;

			Vector4 normalizedColorValue = colorMap.GetTexel(w, h).GetAsFloats();
			m_normalizedColorValues[index] = normalizedColorValue;
		}
	}

	// Second, calculate TBN
	for (u32 h = 0; h < gridHeight; ++h) {
		for (u32 w = 0; w < gridWidth; ++w) {\
			u32 bl_idx = h * gridWidth + w;
			u32 br_idx = h * gridWidth + w + 1;
			u32 tl_idx = (h + 1) * gridWidth + w;
			u32 tr_idx = (h + 1) * gridWidth + w + 1;
			
			Vector3 blPos((float)w, m_heightValues[bl_idx],		(float)h);
			Vector3 brPos((float)w + 1, m_heightValues[br_idx], (float)h);
			Vector3 tlPos((float)w, m_heightValues[tl_idx],		(float)h + 1);
			Vector3 trPos((float)w + 1, m_heightValues[tr_idx], (float)h + 1);

			Vector3 tangent_face1 = (trPos - blPos).GetNormalized();
			Vector3 bitangent_face1 = (tlPos - blPos).GetNormalized();
			Vector3 normal_face1 = CrossProduct(bitangent_face1, tangent_face1).GetNormalized();
			m_normals[bl_idx] += normal_face1;
			m_normals[tl_idx] += normal_face1;
			m_normals[tr_idx] += normal_face1;
			m_tangents[bl_idx] += tangent_face1;
			m_tangents[tl_idx] += tangent_face1;
			m_tangents[tr_idx] += tangent_face1;
			m_bitangents[bl_idx] += bitangent_face1;
			m_bitangents[tl_idx] += bitangent_face1;
			m_bitangents[tr_idx] += bitangent_face1;

			Vector3 tangent_face2 = (brPos - blPos).GetNormalized();
			Vector3 bitangent_face2 = (trPos - blPos).GetNormalized();
			Vector3 normal_face2 = CrossProduct(bitangent_face2, tangent_face2).GetNormalized();
			m_normals[bl_idx] += normal_face2;
			m_normals[br_idx] += normal_face2;
			m_normals[tr_idx] += normal_face2;
			m_tangents[bl_idx] += tangent_face2;
			m_tangents[br_idx] += tangent_face2;
			m_tangents[tr_idx] += tangent_face2;
			m_bitangents[bl_idx] += bitangent_face2;
			m_bitangents[br_idx] += bitangent_face2;
			m_bitangents[tr_idx] += bitangent_face2;
		}
	}

	for(auto& normal : m_normals){
		normal = normal.GetNormalized();
	}
	for(auto& tangent : m_tangents){
		tangent = tangent.GetNormalized();
	}
	for(auto& bitangent : m_bitangents){
		bitangent = bitangent.GetNormalized();
	}

	for (u32 h = 0; h < gridHeight; ++h) {
		for (u32 w = 0; w < gridWidth; ++w) {
			u32 bl_idx = h * gridWidth + w;
			u32 br_idx = h * gridWidth + w + 1;
			u32 tl_idx = (h + 1) * gridWidth + w;
			u32 tr_idx = (h + 1) * gridWidth + w + 1;

			Vector3 blPos((float)w, m_heightValues[bl_idx],		(float)h);
			Vector3 brPos((float)w + 1, m_heightValues[br_idx], (float)h);
			Vector3 tlPos((float)w, m_heightValues[tl_idx],		(float)h + 1);
			Vector3 trPos((float)w + 1, m_heightValues[tr_idx], (float)h + 1);

			Vector2 blUV(0.f, 1.f);
			Vector2 brUV(1.f, 1.f);
			Vector2 tlUV(0.f, 0.f);
			Vector2 trUV(1.f, 0.f);


			Vector3 blNormal = m_normals[bl_idx];
			Vector3 brNormal = m_normals[br_idx];
			Vector3 tlNormal = m_normals[tl_idx];
			Vector3 trNormal = m_normals[tr_idx];

			Vector3 blTangent = m_tangents[bl_idx];
			Vector3 brTangent = m_tangents[br_idx];
			Vector3 tlTangent = m_tangents[tl_idx];
			Vector3 trTangent = m_tangents[tr_idx];

			Vector3 blBitangent = m_bitangents[bl_idx];
			Vector3 brBitangent = m_bitangents[br_idx];
			Vector3 tlBitangent = m_bitangents[tl_idx];
			Vector3 trBitangent = m_bitangents[tr_idx];

			Vector4 blColor = m_normalizedColorValues[bl_idx];
			Vector4 brColor = m_normalizedColorValues[br_idx];
			Vector4 tlColor = m_normalizedColorValues[tl_idx];
			Vector4 trColor = m_normalizedColorValues[tr_idx];

			m_mesh->PushVertex(VertexMaster(blPos, blColor, blUV, blNormal, blTangent, blBitangent));
			m_mesh->PushVertex(VertexMaster(brPos, brColor, brUV, brNormal, brTangent, brBitangent));
			m_mesh->PushVertex(VertexMaster(tlPos, tlColor, tlUV, tlNormal, tlTangent, tlBitangent));
			m_mesh->PushVertex(VertexMaster(trPos, trColor, trUV, trNormal, trTangent, trBitangent));

			u32 startIndex = bl_idx * 4;
			m_mesh->PushTriangle(startIndex, startIndex + 1, startIndex + 2);
			m_mesh->PushTriangle(startIndex + 1, startIndex + 3, startIndex + 2);
		}
	}
}
