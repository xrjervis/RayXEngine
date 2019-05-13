#pragma once
#include "Engine/Renderer/d3d11/Mesh.hpp"
#include "Engine/Core/Image.hpp"
#include <vector>
#include <memory>

class Terrain {
public:
	Terrain();
	~Terrain();

	// 256x256 Terrain needs a 257x257 height map
	void GenerateFromHeightMap(u32 gridWidth, u32 gridHeight, float scale, const Image& heightMap, const Image& colorMap);

public:
	std::vector<float> m_heightValues; 
	std::vector<Vector4> m_normalizedColorValues;
	std::vector<Vector3> m_normals;
	std::vector<Vector3> m_tangents;
	std::vector<Vector3> m_bitangents;
	std::unique_ptr<Mesh<VertexPCUTBN>> m_mesh;
};