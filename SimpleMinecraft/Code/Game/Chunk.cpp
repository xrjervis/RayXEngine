#include "Engine/Math/SmoothNoise.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockLocator.hpp"

Chunk::Chunk(IntVector2 chunkCoords) 
	: m_chunkCoords(chunkCoords) {
}

Chunk::~Chunk() {

}

void Chunk::Update(float deltaSeconds) {

}

void Chunk::Render() const {
	// Only when m_mesh is initialized and has vertex data
	if (m_mesh) {
		if (!m_mesh->m_vertices.empty()) {
			g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(m_mesh.get());
		}
	}
}

void Chunk::DrawOutlineForBlock(int blockIndex, const Vector3& impactNormal) {
	IntVector3 blockCoords = GetBlockCoordsForBlockIndex(blockIndex);
	blockCoords.x += m_chunkCoords.x * CHUNK_SIZE_X;
	blockCoords.y += m_chunkCoords.y * CHUNK_SIZE_Y;
	Vector3 blfCorner(blockCoords);
	blfCorner -= Vector3(0.01f, 0.01f, 0.01f);
	Vector3 brfCorner = blfCorner + Vector3(1.02f, 0.f, 0.f);
	Vector3 tlfCorner = blfCorner + Vector3(0.f, 0.f, 1.02f);
	Vector3 trfCorner = tlfCorner + Vector3(1.02f, 0.f, 0.f);
	Vector3 blbCorner = blfCorner + Vector3(0.f, 1.02f, 0.f);
	Vector3 brbCorner = blbCorner + Vector3(1.02f, 0.f, 0.f);
	Vector3 tlbCorner = blbCorner + Vector3(0.f, 0.f, 1.02f);
	Vector3 trbCorner = tlbCorner + Vector3(1.02f, 0.f, 0.f);

	if (impactNormal == Vector3(1.f, 0.f, 0.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brfCorner, brbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brbCorner, trbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trbCorner, trfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trfCorner, brfCorner, Rgba::PURPLE);
	}
	else if (impactNormal == Vector3(-1.f, 0.f, 0.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blbCorner, blfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blfCorner, tlfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlfCorner, tlbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlbCorner, blbCorner, Rgba::PURPLE);
	}
	else if (impactNormal == Vector3(0.f, 1.f, 0.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brbCorner, blbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blbCorner, tlbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlbCorner, trbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trbCorner, brbCorner, Rgba::PURPLE);
	}
	else if (impactNormal == Vector3(0.f, -1.f, 0.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blfCorner, brfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brfCorner, trfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trfCorner, tlfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlfCorner, blfCorner, Rgba::PURPLE);
	}
	else if (impactNormal == Vector3(0.f, 0.f, 1.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlfCorner, trfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trfCorner, trbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(trbCorner, tlbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(tlbCorner, tlfCorner, Rgba::PURPLE);
	}
	else if (impactNormal == Vector3(0.f, 0.f, -1.f)) {
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blfCorner, brfCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brfCorner, brbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(brbCorner, blbCorner, Rgba::PURPLE);
		g_theRHI->GetImmediateRenderer()->DrawLine3D(blbCorner, blfCorner, Rgba::PURPLE);
	}
}

void Chunk::GenerateBLocks() {
	std::array<float, BLOCKS_PER_LAYER> perlinNoiseValues;
	//--------------------------------------------------------------------
	// Pre-compute perlin noise
	for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
		for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
			int worldX = blockX + m_chunkCoords.x * CHUNK_SIZE_X;
			int worldY = blockY + m_chunkCoords.y * CHUNK_SIZE_Y;
			float noiseValue = Compute2dPerlinNoise((float)worldX, (float)worldY, 300.f, 5, 0.5f, 2.f);
			int blockIndex = blockY * CHUNK_SIZE_X + blockX;
			perlinNoiseValues[blockIndex] = noiseValue;
		}
	}

	for (int blockZ = 0; blockZ < CHUNK_SIZE_Z; ++blockZ) {
		for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
			for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
				int blockIndex = blockY * CHUNK_SIZE_X + blockX;
				float noiseValue = perlinNoiseValues[blockIndex];
				int seaLevel = 120;
				int grassLevel = (int)(noiseValue * 80.f) + seaLevel;
				if (grassLevel > seaLevel) {
					if (blockZ > grassLevel) {
						SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_AIR);
					}
					else if (blockZ == grassLevel) {
						SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_GRASS);
					}
					else if (blockZ < grassLevel && blockZ > seaLevel) {
						SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_DIRT);
					}
				}
				else {
					if (blockZ > seaLevel) {
						SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_AIR);
					}
				}
				if (blockZ <= seaLevel && (seaLevel - blockZ) < 10) {
					SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_ICE);
				}
				else if(blockZ <= seaLevel - 10) {
					SetBlockType(blockX, blockY, blockZ, BLOCK_TYPE_COBBLESTONE);
				}
			}
		}
	}
}

bool Chunk::ValidateFileHeader(char* buffer) const {
	ChunkFileHeader_t validHeader;
	char* headerPtr = reinterpret_cast<char*>(&validHeader);

	for (int i = 0; i < sizeof(validHeader); ++i) {
		if (buffer[i] != *(headerPtr + i)) {
			return false;
		}
	}
	return true;
}

void Chunk::LoadFromFile(const char* filename) {
	char* buffer = FileSystem::FileReadToBuffer(filename);
	//---------------------------------------------------------------
	// Validate file header
	bool succeed = ValidateFileHeader(buffer);
	if (!succeed) {
		delete[] buffer;
		ERROR_AND_DIE(Stringf("[ERROR] Chunk load file failed: %s", filename).c_str());
	}
	int blockIndex = 0;
	int bufferIndex = sizeof(ChunkFileHeader_t);
	while (blockIndex < BLOCKS_PER_CHUNK) {
		u8 runType = buffer[bufferIndex++];
		u8 runCount = buffer[bufferIndex++];
		for (int i = 0; i < runCount; ++i) {
			SetBlockType(blockIndex++, (eBLockType)runType);
		}
	}

	delete[] buffer;
}

void Chunk::SaveToFile() const {
	std::vector<u8> buffer;
	buffer.reserve(2000);
	ChunkFileHeader_t chunkFileHeader;
	u8* headerPtr = reinterpret_cast<u8*>(&chunkFileHeader);
	for (int i = 0; i < sizeof(ChunkFileHeader_t); ++i) {
		buffer.emplace_back(*(headerPtr + i));
	}
	u8 prevRunType = (u8)m_blocks[0].m_type;
	u8 curRunType = prevRunType;
	u8 runCount = 0;

	for (int i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		curRunType = (u8)m_blocks[i].m_type;
		if (curRunType == prevRunType) {
			runCount++;
			if (runCount == 255) {
				buffer.emplace_back(curRunType);
				buffer.emplace_back(runCount);
				runCount = 0;
			}
		}
		else {
			buffer.emplace_back(prevRunType);
			buffer.emplace_back(runCount);
			runCount = 1;
		}
		prevRunType = curRunType;
	}
	buffer.emplace_back(curRunType);
	buffer.emplace_back(runCount);

	FileSystem::BufferWriteToFile(Stringf("Data/Saves/Chunk_%d,%d.chunk", m_chunkCoords.x, m_chunkCoords.y).c_str(), (char*)buffer.data(), buffer.size(), true, false, true);
}

void Chunk::RebuildMesh() {
	IntVector2 a;
	IntVector2 b;
	if (a < b) {
		return;
	}
	if (m_mesh == nullptr) {
		m_mesh = std::make_unique<Mesh<VertexPCU>>(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}
	else {
		m_mesh->Reset(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}

	for (int i = 0; i < BLOCKS_PER_CHUNK; ++i) {
		AddVertsForBlock(i);
	}

	// Add debug quad high up
	m_mesh->AddQuad3D(Vector3(m_chunkCoords.x * CHUNK_SIZE_X, m_chunkCoords.y * CHUNK_SIZE_Y, CHUNK_SIZE_Z * 2), (float)(CHUNK_SIZE_X - 2), (float)(CHUNK_SIZE_Y - 2),
		Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), AABB2());

	m_isMeshDirty = false;
}

void Chunk::AddVertsForBlock(int blockIndex) {
	eBLockType type = m_blocks[blockIndex].m_type;
	if (type == BLOCK_TYPE_AIR) {
		return;
	}
	BlockDefinition* blockDef = BlockDefinition::GetDefinition(type);
	SpriteSheet* spriteSheet = g_theResourceManager->GetSpriteSheet("terrain_32x32");
	AABB2 topUV = spriteSheet->GetUVFromSpriteCoords(blockDef->m_topSpriteCoords);
	AABB2 sideUV = spriteSheet->GetUVFromSpriteCoords(blockDef->m_sideSpriteCoords);
	AABB2 bottomUV = spriteSheet->GetUVFromSpriteCoords(blockDef->m_bottomSpriteCoords);

	IntVector3 blockCoords = GetBlockCoordsForBlockIndex(blockIndex);
	blockCoords.x += m_chunkCoords.x * CHUNK_SIZE_X;
	blockCoords.y += m_chunkCoords.y * CHUNK_SIZE_Y;
	Vector3 blfCorner(blockCoords);
	Vector3 brfCorner = blfCorner + Vector3(1.f, 0.f, 0.f);
	Vector3 tlfCorner = blfCorner + Vector3(0.f, 0.f, 1.f);
	Vector3 trfCorner = tlfCorner + Vector3(1.f, 0.f, 0.f);
	Vector3 blbCorner = blfCorner + Vector3(0.f, 1.f, 0.f);
	Vector3 brbCorner = blbCorner + Vector3(1.f, 0.f, 0.f);
	Vector3 tlbCorner = blbCorner + Vector3(0.f, 0.f, 1.f);
	Vector3 trbCorner = tlbCorner + Vector3(1.f, 0.f, 0.f);

	BlockLocator thisBlock(this, blockIndex);

	//--------------------------------------------------------------------
	// Vertex Color as light color
	// r - indoor; g - outdoor


	//--------------------------------------------------------------------
	// Hidden surface removable
	//south
	{
		BlockLocator southBlock = thisBlock.GetBlockLocatorToSouth();
		eBLockType southBlockType = southBlock.m_chunk->GetBlockType(southBlock.m_blockIndex);
		const Block& b = southBlock.m_chunk->m_blocks[southBlock.m_blockIndex];

		if (southBlockType == BLOCK_TYPE_AIR ) {
			Rgba vertexColor;
			vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
			vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
			vertexColor.b = 127;
			m_mesh->AddQuad3D(blfCorner, 1.f, 1.f, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), sideUV, vertexColor);
		}
	}

	//east
	{
		BlockLocator eastBlock = thisBlock.GetBlockLocatorToEast();
		eBLockType eastBlockType = eastBlock.m_chunk->GetBlockType(eastBlock.m_blockIndex);
		const Block& b = eastBlock.m_chunk->m_blocks[eastBlock.m_blockIndex];

		if (eastBlockType == BLOCK_TYPE_AIR) {
			Rgba vertexColor;
			vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
			vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
			vertexColor.b = 127;
			m_mesh->AddQuad3D(brfCorner, 1.f, 1.f, Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), sideUV, vertexColor);
		}
	}

	//north
	{
		BlockLocator northBlock = thisBlock.GetBlockLocatorToNorth();
		eBLockType northBlockType = northBlock.m_chunk->GetBlockType(northBlock.m_blockIndex);
		const Block& b = northBlock.m_chunk->m_blocks[northBlock.m_blockIndex];

		if (northBlockType == BLOCK_TYPE_AIR ) {
			Rgba vertexColor;
			vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
			vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
			vertexColor.b = 127;
			m_mesh->AddQuad3D(brbCorner, 1.f, 1.f, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), sideUV, vertexColor);
		}
	}

	//west
	{
		BlockLocator westBlock = thisBlock.GetBlockLocatorToWest();
		eBLockType westBlockType = westBlock.m_chunk->GetBlockType(westBlock.m_blockIndex);
		const Block& b = westBlock.m_chunk->m_blocks[westBlock.m_blockIndex];

		if (westBlockType == BLOCK_TYPE_AIR ) {
			Rgba vertexColor;
			vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
			vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
			vertexColor.b = 127;
			m_mesh->AddQuad3D(blbCorner, 1.f, 1.f, Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), sideUV, vertexColor);
		}
	}

	//above
	{
		BlockLocator aboveBlock = thisBlock.GetBlockLocatorAbove();
		if (aboveBlock.m_chunk) {
			eBLockType aboveBlockType = aboveBlock.m_chunk->GetBlockType(aboveBlock.m_blockIndex);
			const Block& b = aboveBlock.m_chunk->m_blocks[aboveBlock.m_blockIndex];

			if (aboveBlockType == BLOCK_TYPE_AIR) {
				Rgba vertexColor;
				vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
				vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
				vertexColor.b = 127;
				m_mesh->AddQuad3D(tlfCorner, 1.f, 1.f, Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), topUV, vertexColor);
			}
		}
	}

	//below
	{
		BlockLocator belowBlock = thisBlock.GetBlockLocatorBelow();
		if (belowBlock.m_chunk) {
			eBLockType belowBlockType = belowBlock.m_chunk->GetBlockType(belowBlock.m_blockIndex);
			const Block& b = belowBlock.m_chunk->m_blocks[belowBlock.m_blockIndex];

			if (belowBlockType == BLOCK_TYPE_AIR) {
				Rgba vertexColor;
				vertexColor.r = RangeMapChar(b.GetIndoorLightLevel(), 0, 15, 0, 255);
				vertexColor.g = RangeMapChar(b.GetOutdoorLightLevel(), 0, 15, 0, 255);
				vertexColor.b = 127;
				m_mesh->AddQuad3D(brfCorner, 1.f, 1.f, Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), bottomUV, vertexColor);
			}
		}
	}
}
