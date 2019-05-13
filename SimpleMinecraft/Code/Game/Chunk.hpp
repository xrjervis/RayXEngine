#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Renderer/d3d11/Mesh.hpp"
#include "Game/Block.hpp"
#include <array>

constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Y = 16;
constexpr int CHUNK_SIZE_Z = 256;
		  
constexpr int CHUNK_BITS_X = 4;
constexpr int CHUNK_BITS_Y = 4;
constexpr int CHUNK_BITS_Z = 8;

constexpr int CHUNK_MASK_X = CHUNK_SIZE_X - 1;
constexpr int CHUNK_MASK_Y = (CHUNK_SIZE_Y - 1) << CHUNK_BITS_X;
constexpr int CHUNK_MASK_Z = (CHUNK_SIZE_Z - 1) << (CHUNK_BITS_X + CHUNK_BITS_Y);
		  
constexpr int BLOCKS_PER_CHUNK = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;
constexpr int BLOCKS_PER_LAYER = CHUNK_SIZE_X * CHUNK_SIZE_Y;

struct ChunkFileHeader_t {
	u8 m_4cc[4]			= { 'S', 'M', 'C', 'D' };	// Unique 4CC (SimpleMiner Chunk Data)
	u8 m_version		= 1;						// .chunk file format version #(1)
	u8 m_chunkBitsX		= (u8)CHUNK_BITS_X;
	u8 m_chunkBitsY		= (u8)CHUNK_BITS_Y;
	u8 m_chunkBitsZ		= (u8)CHUNK_BITS_Z;
	u8 m_reserved1		= 0;
	u8 m_reserved2		= 0;
	u8 m_reserved3		= 0;
	u8 m_format			= 'R';
};

class Chunk {
	friend class World;
	friend class BlockLocator;
public:
	Chunk(IntVector2 chunkCoords);
	~Chunk();

	void			Update(float deltaSeconds);
	void			Render() const;
	void			DrawOutlineForBlock(int blockIndex, const Vector3& impactNormal);

	int				GetBlockIndexForBlockCoords(const IntVector3& blockCoords) const;
	IntVector3		GetBlockCoordsForBlockIndex(int blockIndex) const;
	IntVector3		GetBlockCoordsInWorldFromBlockIndexInChunk(int blockIndex) const;

	void			SetBlockType(int x, int y, int z, eBLockType type);
	void			SetBlockType(int blockIndex, eBLockType type);

	void			GenerateBLocks();
	bool			ValidateFileHeader(char* buffer) const;
	void			LoadFromFile(const char* filename);
	void			SaveToFile() const;
	void			RebuildMesh();
	void			AddVertsForBlock(int blockIndex);

	Chunk*			GetEastNeighbor() const;
	Chunk*			GetWestNeighbor() const;
	Chunk*			GetNorthNeighbor() const;
	Chunk*			GetSouthNeightbor() const;

	eBLockType		GetBlockType(int blockIndex) const;
	bool			HasFourNeighbors() const;
	bool			HasMeshData() const;

private:
	IntVector2		m_chunkCoords;
	std::array<Block, BLOCKS_PER_CHUNK> m_blocks;
	Uptr<Mesh<VertexPCU>>		m_mesh;

	bool			m_isMeshDirty = true;
	bool			m_needsSaving = false;

	Chunk*			m_eastNeighbor = nullptr;
	Chunk*			m_westNeighbor = nullptr;
	Chunk*			m_northNeighbor = nullptr;
	Chunk*			m_southNeighbor = nullptr;
};

inline int Chunk::GetBlockIndexForBlockCoords(const IntVector3& blockCoords) const {
	// 4 cycles version
	return (blockCoords.x) | (blockCoords.y << CHUNK_BITS_X) | (blockCoords.z << (CHUNK_BITS_X + CHUNK_BITS_Y));
}

inline IntVector3 Chunk::GetBlockCoordsForBlockIndex(int blockIndex) const {
	// 4 cycles version
	int z = blockIndex >> (CHUNK_BITS_X + CHUNK_BITS_Y);
	int y = (blockIndex >> CHUNK_BITS_X) & (CHUNK_SIZE_Y - 1);
	int x = blockIndex & (CHUNK_SIZE_X - 1);
	return IntVector3(x, y, z);
}

inline IntVector3 Chunk::GetBlockCoordsInWorldFromBlockIndexInChunk(int blockIndex) const {
	IntVector3 blockCoordsInChunk = GetBlockCoordsForBlockIndex(blockIndex);
	
	return (IntVector3(m_chunkCoords.x * CHUNK_SIZE_X, m_chunkCoords.y * CHUNK_SIZE_Y, 0) + blockCoordsInChunk);
}

inline void Chunk::SetBlockType(int x, int y, int z, eBLockType type) {
	int blockIndex = GetBlockIndexForBlockCoords(IntVector3(x, y, z));
	SetBlockType(blockIndex, type);
}

inline void Chunk::SetBlockType(int blockIndex, eBLockType type) {
	m_blocks[blockIndex].m_type = type;

	// Store bitFlags on block for convenience
	m_blocks[blockIndex].m_bitFlags = BlockDefinition::GetDefinition(type)->m_bitFlags;
}

inline Chunk* Chunk::GetEastNeighbor() const {
	return m_eastNeighbor;
}

inline Chunk* Chunk::GetWestNeighbor() const {
	return m_westNeighbor;
}

inline Chunk* Chunk::GetNorthNeighbor() const {
	return m_northNeighbor;
}

inline Chunk* Chunk::GetSouthNeightbor() const {
	return m_southNeighbor;
}

inline eBLockType Chunk::GetBlockType(int blockIndex) const {
	return m_blocks[blockIndex].m_type;
}

inline bool Chunk::HasFourNeighbors() const {
	return m_eastNeighbor && m_westNeighbor && m_northNeighbor && m_southNeighbor;
}

inline bool Chunk::HasMeshData() const {
	if (m_mesh && m_mesh->GetVerticesCount() != 0) {
		return true;
	}
	else {
		return false;
	}
}