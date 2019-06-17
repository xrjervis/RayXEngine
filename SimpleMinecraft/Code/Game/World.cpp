#include "Engine/Core/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/SmoothNoise.hpp"
#include "Game/World.hpp"
#include "Game/TheApp.hpp"
#include "Game/Player.hpp"
#include "Game/GameCamera.hpp"

World::World() {
	//--------------------------------------------------------------------------
	// Create the main 2d camera
	m_mainCamera2D = std::make_unique<Camera>();
	m_mainCamera2D->SetViewport(0U, 0, 0, g_mainOutput->GetWidth(), g_mainOutput->GetHeight(), 0.f, 1.f);
	m_mainCamera2D->SetProjectionMode(ORTHOGRAPHIC);
	m_mainCamera2D->SetOrtho(Vector2::ZERO, Vector2(g_mainOutput->GetWidth(), g_mainOutput->GetHeight()), 0.f, 10.f);
	m_mainCamera2D->SetRenderTarget(g_mainOutput->GetRTV());
	m_mainCamera2D->SetDepthTarget(g_mainOutput->GetDSV());

	//--------------------------------------------------------------------------
	// Create the main 3d camera
	m_mainCamera3D = std::make_unique<GameCamera>();
	m_mainCamera3D->SetViewport(0U, 0, 0, g_mainOutput->GetWidth(), g_mainOutput->GetHeight(), 0.f, 1.f);
	m_mainCamera3D->SetProjectionMode(PERSPECTIVE);
	m_mainCamera3D->SetPerspective(45.f, 16.f / 9.f, 0.1f, 2000.f);
	m_mainCamera3D->SetRenderTarget(g_mainOutput->GetRTV());
	m_mainCamera3D->SetDepthTarget(g_mainOutput->GetDSV());

	//--------------------------------------------------------------------------
	// Setup debug draw system to use my 2d/3d cameras
	g_theDebugDrawSystem->SetCamera2D(m_mainCamera2D.get());
	g_theDebugDrawSystem->SetCamera3D(dynamic_cast<Camera*>(m_mainCamera3D.get()));

	//--------------------------------------------------------------------------
	// Debug draw base close to origin point, leaving a gap of (1,1,1) to avoid
	// overlapping with the Block edge
	DebugDrawBase(9999.f, Vector3(-.5f, -.5f, -.5f), m_mainCamera3D->m_transform.GetRight(), m_mainCamera3D->m_transform.GetUp(), m_mainCamera3D->m_transform.GetForward(), 10.f);

	DebugString(10.f, "World Begin!", Rgba::GREEN, Rgba::RED);

	//--------------------------------------------------------------------------
	// Load all block definitions (hard coded for now)
	BlockDefinition::LoadDefinitions();

	//--------------------------------------------------------------------------
	// Activate chunk hard coded for now as well
	ActivateChunk(IntVector2(0,	0));
	ActivateChunk(IntVector2(1, 0));
	ActivateChunk(IntVector2(0, 1));
	ActivateChunk(IntVector2(-1, 0));
	ActivateChunk(IntVector2(0, -1));

	//--------------------------------------------------------------------
	// Set fog range
	g_theRHI->GetImmediateRenderer()->SetFogStart((float)g_gameConfig->GetValue("ChunkActivationRange", 20) * 0.5f);
	g_theRHI->GetImmediateRenderer()->SetFogEnd((float)g_gameConfig->GetValue("ChunkActivationRange", 20) - 16.f);

	//--------------------------------------------------------------------
	// Set default indoor and outdoor color
	m_baseIndoorColor = Rgba(1.0f, 0.9f, 0.8f);
	m_baseOutdoorColor = Rgba(0.8f, 0.9f, 1.0f);
	g_theRHI->GetImmediateRenderer()->SetIndoorColor(m_baseIndoorColor);
	g_theRHI->GetImmediateRenderer()->SetOutdoorColor(m_baseOutdoorColor);

	//--------------------------------------------------------------------
	// Spawn player and camera
	m_player = std::make_unique<Player>(this);
	m_player->m_transform.SetWorldPosition(Vector3(-CHUNK_SIZE_X * 0.5f, 160.f, CHUNK_SIZE_Y * 0.5f));
	if (m_player->m_colliderType == COLLIDER_TYPE_UNIBALL) {
		m_mainCamera3D->m_transform.LookAt(Vector3(0.f, 2.5f, -8.f), Vector3(0.f, 0.f, 0.f));
	}
	else {
		m_mainCamera3D->m_transform.LookAt(Vector3(0.f, 2.5f, -8.f), Player::s_eyePosition);
	}
	m_mainCamera3D->m_transform.SetParent(&m_player->m_transform);
}

World::~World() {
	DeactivateAllChunks();
}

void World::Update(float deltaSeconds) {
	// Some debug info
	if (g_theApp->m_isDebugMode) {
		DebugString(0.f, Stringf("Range: %dm,  Active chunks: %d", g_gameConfig->GetValue("ChunkActivationRange", 20), m_activeChunks.size()), Rgba::YELLOW, Rgba::YELLOW);
		DebugString(0.f, Stringf("World time: %.2f ", m_worldTime), Rgba::YELLOW, Rgba::YELLOW);
		DebugString(0.f, "Debug mode: On!", Rgba::YELLOW, Rgba::YELLOW);
	}

	UpdateInput(deltaSeconds);

	// Update Physics
	m_player->Update(deltaSeconds);
	ApplyNewtonianPhysics(deltaSeconds);
	ApplyCorrectiveCollisionDetection();

	// Day and night cycles
	UpdateWorldTime(deltaSeconds);
	ApplyLightningStrikeToSkyColor();
	ApplyGlowstoneFlickerToIndoorLight();

	// Update chunks
	for (auto& it : m_activeChunks) {
		it.second->Update(deltaSeconds);
	}

	// Update lighting
	UpdateDirtyLighting();

	// Manage chunks
	Vector3 pos = GetCameraCurrentPosition();
	Vector3 forward = m_mainCamera3D->m_transform.GetForward();
	Vector3 dir = (Vector4(forward, 0.f) * Matrix44::EngineToGame).xyz().GetNormalized();
	//m_raycastResult = StepAndSampleRaycast(pos, dir, 8.f);
	m_raycastResult = FastVoxelRaycast(pos, dir, 12.f);

	FindAndActivateNearestChunkInRange();

	FindAndDeactivateFarthestChunkOutRange();

	FindAndRebuildChunkMesh();
}

void World::Render() const {
	//--------------------------------------------------------------------------
	// Setup render pipeline
	g_theRHI->GetImmediateRenderer()->BindCamera(m_mainCamera2D.get());
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetDevice()->ClearColor(g_mainOutput->GetRTV(), m_skyColor);
	g_theRHI->GetDevice()->ClearDepthStencil(g_mainOutput->GetDSV(), 1.f, 0U);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(m_mainCamera2D->GetOrtho().GetCenter(), Vector2(0.5f, 0.5f), Vector2(5.f, 5.f));


	g_theRHI->GetImmediateRenderer()->BindCamera(dynamic_cast<Camera*>(m_mainCamera3D.get()));
	g_theRHI->GetImmediateRenderer()->DrawSkybox();

	//--------------------------------------------------------------------
	RenderChunks();

	//--------------------------------------------------------------------
	m_player->Render();


	//--------------------------------------------------------------------
	if (m_raycastResult.DidImpact()) {
		BlockLocator impactBlock = m_raycastResult.m_impactBlock;
		g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("debug"));
		impactBlock.m_chunk->DrawOutlineForBlock(impactBlock.m_blockIndex, m_raycastResult.m_impactNormal);
	}

// 	if (g_theApp->m_isDebugMode) {
// 		RenderDebugPointsForDirtyLightingBlocks();
// 	}
}

Chunk* World::ActivateChunk(const IntVector2& chunkCoords) {
	//--------------------------------------------------------------------
	// New chunk
	Uptr<Chunk> newChunk = std::make_unique<Chunk>(chunkCoords);

	//--------------------------------------------------------------------
	// Generate or Load blocks
	std::string chunkFileName = Stringf("Data/Saves/Chunk_%d,%d.chunk", chunkCoords.x, chunkCoords.y);
	if (FileSystem::Exists(chunkFileName)) {
		newChunk->LoadFromFile(chunkFileName.c_str());
	}
	else {
		newChunk->GenerateBLocks();
	}

	//--------------------------------------------------------------------
	// Add to world
	Chunk* chunkPtr = newChunk.get();
	m_activeChunks.insert({ chunkCoords, std::move(newChunk) });

	//--------------------------------------------------------------------
	// Link up neighbors
	LinkUpChunkNeighbor(chunkCoords);

	//--------------------------------------------------------------------
	//Mark non-opaque edge blocks touching any existing neighboring chunk (NSEW) as dirty
	for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
		for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
			if (blockX == 0 || blockY == 0 || blockX == CHUNK_SIZE_X - 1 || blockY == CHUNK_SIZE_Y - 1) {
				for (int blockZ = CHUNK_SIZE_Z - 1; blockZ >= 0; --blockZ) {
					int blockIndex = chunkPtr->GetBlockIndexForBlockCoords(IntVector3(blockX, blockY, blockZ));
					BlockLocator bl(chunkPtr, blockIndex);
					Block& b = chunkPtr->m_blocks[blockIndex];
					if (!b.IsFullOpaque()) {
						MarkBlockLightingDirty(bl);
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------
	// Mark sky, mark blocks emit light dirty
	for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
		for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
			for (int blockZ = CHUNK_SIZE_Z - 1; blockZ >= 0; --blockZ) {
				int blockIndex = chunkPtr->GetBlockIndexForBlockCoords(IntVector3(blockX, blockY, blockZ));
				BlockLocator bl(chunkPtr, blockIndex);
				Block& b = chunkPtr->m_blocks[blockIndex];
				if (b.IsFullOpaque()) {
					break;
				}
				else {
					b.SetIsBlockSky();
				}
			}
		}
	}

	//--------------------------------------------------------------------
	// Set outdoor lighting level to 15, and mark its horizontal non-opaque, non-sky block dirty
	for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
		for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
			for (int blockZ = CHUNK_SIZE_Z - 1; blockZ >= 0; --blockZ) {
				int blockIndex = chunkPtr->GetBlockIndexForBlockCoords(IntVector3(blockX, blockY, blockZ));
				BlockLocator bl(chunkPtr, blockIndex);
				Block& b = chunkPtr->m_blocks[blockIndex];
				if (b.IsFullOpaque()) {
					break;
				}
				else if(b.IsBlockSky()) {
					b.SetOutDoorLightLevel(15U);
					BlockLocator eastBL = bl.GetBlockLocatorToEast();
					const Block& eastBlock = eastBL.m_chunk->m_blocks[eastBL.m_blockIndex];
					if (eastBL.m_chunk && !eastBlock.IsBlockSky() && !eastBlock.IsFullOpaque()) {
						MarkBlockLightingDirty(eastBL);
					}
					BlockLocator westBL = bl.GetBlockLocatorToWest();
					const Block& westBlock = westBL.m_chunk->m_blocks[westBL.m_blockIndex];
					if (westBL.m_chunk && !westBlock.IsBlockSky() && !westBlock.IsFullOpaque()) {
						MarkBlockLightingDirty(westBL);
					}
					BlockLocator northBL = bl.GetBlockLocatorToNorth();
					const Block& northBlock = northBL.m_chunk->m_blocks[northBL.m_blockIndex];
					if (northBL.m_chunk && !northBlock.IsBlockSky() && !northBlock.IsFullOpaque()) {
						MarkBlockLightingDirty(northBL);
					}
					BlockLocator southBL = bl.GetBlockLocatorToSouth();
					const Block& southBlock = southBL.m_chunk->m_blocks[southBL.m_blockIndex]; 
					if (southBL.m_chunk && !southBlock.IsBlockSky() && !southBlock.IsFullOpaque()) {
						MarkBlockLightingDirty(southBL);
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------
	// Loop through all blocks, mark self-emission blocks dirty
	for (int blockY = 0; blockY < CHUNK_SIZE_Y; ++blockY) {
		for (int blockX = 0; blockX < CHUNK_SIZE_X; ++blockX) {
			for (int blockZ = CHUNK_SIZE_Z - 1; blockZ >= 0; --blockZ) {
				int blockIndex = chunkPtr->GetBlockIndexForBlockCoords(IntVector3(blockX, blockY, blockZ));
				BlockLocator bl(chunkPtr, blockIndex);
				Block& b = chunkPtr->m_blocks[blockIndex];
				if (BlockDefinition::GetDefinition(b.m_type)->m_indoorLightLevel > 0) {
					MarkBlockLightingDirty(bl);
				}
			}
		}
	}

	return chunkPtr;
}

void World::DeactivateChunk(const IntVector2& chunkCoords) {
	Chunk* chunk = m_activeChunks.at(chunkCoords).get();
	if (chunk->m_eastNeighbor) {
		chunk->m_eastNeighbor->m_westNeighbor = nullptr;
	}
	if (chunk->m_westNeighbor) {
		chunk->m_westNeighbor->m_eastNeighbor = nullptr;
	}
	if (chunk->m_northNeighbor) {
		chunk->m_northNeighbor->m_southNeighbor = nullptr;
	}
	if (chunk->m_southNeighbor) {
		chunk->m_southNeighbor->m_northNeighbor = nullptr;
	}

	// Needs saving?
	if (chunk->m_needsSaving) {
		chunk->SaveToFile();
	}

	m_activeChunks.erase(chunkCoords);
}

void World::DeactivateAllChunks() {
	while (!m_activeChunks.empty()) {
		IntVector2 chunkCoords = m_activeChunks.begin()->first;
		DeactivateChunk(chunkCoords);
	}
}

void World::RenderChunks() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("smc"));

	//--------------------------------------------------------------------
	// For each chunk, Render!
	{
		for (auto& it : m_activeChunks) {
			it.second->Render();
		}
	}
}

IntVector2 World::GetCameraCurrentChunkCoords() const {
	Vector3 cameraPosition = m_mainCamera3D->m_transform.GetWorldPosition();
	Vector3 myPosition(cameraPosition.z, -cameraPosition.x, cameraPosition.y);
	IntVector2 chunkCoords((int)(myPosition.x / (float)CHUNK_SIZE_X), (int)(myPosition.y / (float)CHUNK_SIZE_Y));
	return chunkCoords;
}

Vector3 World::GetCameraCurrentPosition() const {
	Vector3 cameraPosition = m_mainCamera3D->m_transform.GetWorldPosition();
	return Vector3(cameraPosition.z, -cameraPosition.x, cameraPosition.y);
}


IntVector3 World::GetBlockCoordsInChunkFromWorld(const IntVector3& blockCoordsInWorld) const {
	IntVector3 blockCoordsInChunk(blockCoordsInWorld.x % CHUNK_SIZE_X, blockCoordsInWorld.y % CHUNK_SIZE_Y, blockCoordsInWorld.z);
	while (blockCoordsInChunk.x < 0) {
		blockCoordsInChunk.x += CHUNK_SIZE_X;
	}
	while (blockCoordsInChunk.y < 0) {
		blockCoordsInChunk.y += CHUNK_SIZE_Y;
	}
	return blockCoordsInChunk;
}

IntVector3 World::GetBlockCoordsInWorld(const Vector3& position) const {
	return IntVector3((int)floorf(position.x), (int)floorf(position.y), (int)floorf(position.z));
}

Chunk* World::GetChunkForBlockCoordsInWorld(const IntVector3& blockCoordsInWorld) const {
	IntVector2 chunkCoords;
	if (blockCoordsInWorld.x < 0) {
		chunkCoords.x = ((blockCoordsInWorld.x + 1) / CHUNK_SIZE_X) - 1;
	}
	else {
		chunkCoords.x = blockCoordsInWorld.x / CHUNK_SIZE_X;
	}

	if (blockCoordsInWorld.y < 0) {
		chunkCoords.y = ((blockCoordsInWorld.y + 1) / CHUNK_SIZE_Y) - 1;
	}
	else {
		chunkCoords.y = blockCoordsInWorld.y / CHUNK_SIZE_Y;
	}

	if (m_activeChunks.find(chunkCoords) == m_activeChunks.end()) {
		// if chunk doesn't exist (raycast if out of range), return nullptr
		return nullptr;
	}
	else {
		Chunk* chunk = m_activeChunks.at(chunkCoords).get();
		return chunk;
	}
}

void World::FindAndActivateNearestChunkInRange() {
	IntVector2 myChunkCoords = GetCameraCurrentChunkCoords();
	Vector2 myChunkPosition(myChunkCoords.x * CHUNK_SIZE_X, myChunkCoords.y * CHUNK_SIZE_Y);
	IntVector2 nearestMissingChunkCoords = myChunkCoords;
	int activationRange = g_gameConfig->GetValue("ChunkActivationRange", 20);
	int minDistanceSquared = activationRange * activationRange;
	
	for (int x = myChunkCoords.x - activationRange; x < myChunkCoords.x + activationRange; ++x) {
		for (int y = myChunkCoords.y - activationRange; y < myChunkCoords.y + activationRange; ++y) {
			Vector2 chunkPosition(x * CHUNK_SIZE_X, y * CHUNK_SIZE_Y);
			int distanceSquared = (chunkPosition - myChunkPosition).GetLengthSquared();
			if (distanceSquared <= activationRange * activationRange && distanceSquared <= minDistanceSquared) {
				IntVector2 chunkCoords(x, y);
				if (m_activeChunks.find(chunkCoords) == m_activeChunks.end()) {
					nearestMissingChunkCoords = chunkCoords;
					minDistanceSquared = distanceSquared;
				}
			}
		}
	}

	if (nearestMissingChunkCoords != myChunkCoords) {
		ActivateChunk(nearestMissingChunkCoords);
	}
}

void World::FindAndDeactivateFarthestChunkOutRange() {
	IntVector2 myChunkCoords = GetCameraCurrentChunkCoords();
	Vector2 myChunkPosition(myChunkCoords.x * CHUNK_SIZE_X, myChunkCoords.y * CHUNK_SIZE_Y);
	int activationRange = g_gameConfig->GetValue("ChunkActivationRange", 20) + 16;
	int maxDistanceSquared = 0;
	IntVector2 farthestChunkCoordsToDelete = myChunkCoords;

	for (auto& it : m_activeChunks) {
		IntVector2 chunkCoords = it.first;
		Vector2 chunkPosition(chunkCoords.x * CHUNK_SIZE_X, chunkCoords.y * CHUNK_SIZE_Y);
		int distanceSquared = (chunkPosition - myChunkPosition).GetLengthSquared();
		if (distanceSquared > activationRange * activationRange && distanceSquared > maxDistanceSquared) {
			farthestChunkCoordsToDelete = chunkCoords;
			maxDistanceSquared = distanceSquared;
		}
	}
	if (farthestChunkCoordsToDelete != myChunkCoords) {
		DeactivateChunk(farthestChunkCoordsToDelete);
	}
}

void World::FindAndRebuildChunkMesh() const {
	int counter = 0;
	for (auto& it : m_activeChunks) {
		Chunk* chunk = it.second.get();
		if (chunk->m_isMeshDirty) {
			if (chunk->HasFourNeighbors()) {
				// 2 chunks per frame
				chunk->RebuildMesh();
				counter++;
				if (counter == 2) {
					break;
				}
			}
		}
	}
}

void World::LinkUpChunkNeighbor(const IntVector2& chunkCoords) const {
	Chunk* thisChunk = m_activeChunks.at(chunkCoords).get();

	IntVector2 eastCoords = chunkCoords + IntVector2(1, 0);
	if (m_activeChunks.find(eastCoords) != m_activeChunks.end()) {
		Chunk* eastChunk = m_activeChunks.at(eastCoords).get();
		thisChunk->m_eastNeighbor = eastChunk;
		eastChunk->m_westNeighbor = thisChunk;
	}
	IntVector2 westCoords = chunkCoords + IntVector2(-1, 0);
	if (m_activeChunks.find(westCoords) != m_activeChunks.end()) {
		Chunk* westChunk = m_activeChunks.at(westCoords).get();
		thisChunk->m_westNeighbor = westChunk;
		westChunk->m_eastNeighbor = thisChunk;
	}
	IntVector2 northCoords = chunkCoords + IntVector2(0, 1);
	if (m_activeChunks.find(northCoords) != m_activeChunks.end()) {
		Chunk* northChunk = m_activeChunks.at(northCoords).get();
		thisChunk->m_northNeighbor = northChunk;
		northChunk->m_southNeighbor = thisChunk;
	}
	IntVector2 southCoords = chunkCoords + IntVector2(0, -1);
	if (m_activeChunks.find(southCoords) != m_activeChunks.end()) {
		Chunk* southChunk = m_activeChunks.at(southCoords).get();
		thisChunk->m_southNeighbor = southChunk;
		southChunk->m_northNeighbor = thisChunk;
	}
}

void World::UpdateInput(float deltaSeconds) {

	//--------------------------------------------------------------------
	// Debug key U
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_U)) {
		DeactivateAllChunks();
		IntVector2 currentChunkCoords = GetCameraCurrentChunkCoords();
		ActivateChunk(currentChunkCoords);
	}

	//--------------------------------------------------------------------
	// Debug key T
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_T)) {
		m_timeScale = 10000.f;
	}
	else {
		m_timeScale = 200.f;
	}

	// Mouse Mode Key M
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_M)) {
		if (g_theInput->GetMouseMode() == MOUSEMODE_FREE) {
			g_theInput->SetMouseMode(MOUSEMODE_SNAP);
		}
		else {
			g_theInput->SetMouseMode(MOUSEMODE_FREE);
		}
	}

	//--------------------------------------------------------------------
	// Dig and place block
	if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_LEFT)) {
		Dig();
	}

	if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_RIGHT)) {
		if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_CTRL)) {
			Place(BLOCK_TYPE_GLOWSTONE);
		}
		else {
			Place(BLOCK_TYPE_COBBLESTONE);
		}
	}
}

RaycastResult3D_t World::StepAndSampleRaycast(const Vector3& startPos, const Vector3& forwardNormal, float maxDistance) const {
	RaycastResult3D_t result;
	// set to default miss
	result.m_impactPos = startPos + maxDistance * forwardNormal;
	result.m_startPos = startPos;
	result.m_direction = forwardNormal;
	result.m_maxDistance = maxDistance;
	result.m_endPos = startPos + maxDistance * forwardNormal;
	result.m_impactFraction = 1.f;
	result.m_impactDistance = maxDistance;

	// check start block is solid
	IntVector3 startBlockCoords((int)floorf(startPos.x), (int)floorf(startPos.y), (int)floorf(startPos.z));
	IntVector2 startChunkCoords(startBlockCoords.x / CHUNK_SIZE_X, startBlockCoords.y / CHUNK_SIZE_Y);
	if (startBlockCoords.x < 0) {
		startChunkCoords.x--;
	}
	if (startBlockCoords.y < 0) {
		startChunkCoords.y--;
	}

	Chunk* startChunk = m_activeChunks.at(startChunkCoords).get();

	IntVector3 startBlockCoordsInChunk(startBlockCoords.x % CHUNK_SIZE_X, startBlockCoords.y % CHUNK_SIZE_Y, startBlockCoords.z);
	while (startBlockCoordsInChunk.x < 0) {
		startBlockCoordsInChunk.x += CHUNK_SIZE_X;
	}
	while (startBlockCoordsInChunk.y < 0) {
		startBlockCoordsInChunk.y += CHUNK_SIZE_Y;
	}

	int startBlockIndexInChunk = startChunk->GetBlockIndexForBlockCoords(startBlockCoordsInChunk);
	BlockLocator startBlock(startChunk, startBlockIndexInChunk);
	eBLockType startBlockType = startBlock.GetType();

// 	DebugString(0.f, Stringf("[Raycast] Start block coords: %d, %d, %d", startBlockCoords.x, startBlockCoords.y, startBlockCoords.z), Rgba::RED, Rgba::RED);
// 	DebugString(0.f, Stringf("[Raycast] Start chunk coords: %d, %d", startChunk->m_chunkCoords.x, startChunk->m_chunkCoords.y), Rgba::RED, Rgba::RED);
// 	DebugString(0.f, Stringf("[Raycast] Start block coords in chunk: %d, %d, %d", startBlockCoordsInChunk.x, startBlockCoordsInChunk.y, startBlockCoordsInChunk.z), Rgba::RED, Rgba::RED);
// 	DebugString(0.f, Stringf("[Raycast] Start block index in chunk: %d", startBlockIndexInChunk), Rgba::RED, Rgba::RED);

	if (startBlockType != BLOCK_TYPE_AIR) {
		result.m_impactBlock = startBlock;
		result.m_impactPos = startPos;
		result.m_impactNormal = -forwardNormal;
		result.m_startPos = startPos;
		result.m_direction = forwardNormal;
		result.m_maxDistance = maxDistance;
		result.m_endPos = startPos;
		result.m_impactFraction = 0.f;
		result.m_impactDistance = 0.f;
		DebugString(0.f, "[Raycast] Start is solid!", Rgba::RED, Rgba::RED);
		return result;
	}

	BlockLocator prevBlock = startBlock;
	IntVector3 prevBlockCoords = startBlockCoords;
	int stepsPerMeter = g_gameConfig->GetValue("StepsPerMeter", 10);
	int numSteps = stepsPerMeter * maxDistance;
	for (int i = 0; i < numSteps; ++i) {
		Vector3 newPos = startPos + (((float)i / numSteps) * maxDistance) * forwardNormal;
		IntVector3 newBlockCoords((int)floorf(newPos.x), (int)floorf(newPos.y), (int)floorf(newPos.z));
		if (newBlockCoords == prevBlockCoords) {
			continue;
		}
		else {
			if (newBlockCoords.x != prevBlockCoords.x) {
				if (newBlockCoords.x > prevBlockCoords.x) {
					BlockLocator newBlock = prevBlock.GetBlockLocatorToEast();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(-1.f, 0.f, 0.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.x = newBlockCoords.x;
					prevBlock = newBlock;
				}
				else {
					BlockLocator newBlock = prevBlock.GetBlockLocatorToWest();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(1.f, 0.f, 0.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.x = newBlockCoords.x;
					prevBlock = newBlock;
				}
			}
			if (newBlockCoords.y != prevBlockCoords.y) {
				if (newBlockCoords.y > prevBlockCoords.y) {
					BlockLocator newBlock = prevBlock.GetBlockLocatorToNorth();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(0.f, -1.f, 0.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.y = newBlockCoords.y;
					prevBlock = newBlock;
				}
				else {
					BlockLocator newBlock = prevBlock.GetBlockLocatorToSouth();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(0.f, 1.f, 0.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.y = newBlockCoords.y;
					prevBlock = newBlock;
				}
			}
			if (newBlockCoords.z != prevBlockCoords.z) {
				if (newBlockCoords.z > prevBlockCoords.z) {
					BlockLocator newBlock = prevBlock.GetBlockLocatorAbove();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(0.f, 0.f, -1.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.z = newBlockCoords.z;
					prevBlock = newBlock;
				}
				else {
					BlockLocator newBlock = prevBlock.GetBlockLocatorBelow();
					eBLockType type = newBlock.GetType();
					if (type != BLOCK_TYPE_AIR) {
						result.m_impactBlock = newBlock;
						result.m_impactNormal = Vector3(0.f, 0.f, 1.f);
						result.m_impactPos = newPos;
						result.m_startPos = startPos;
						result.m_direction = forwardNormal;
						result.m_maxDistance = maxDistance;
						result.m_endPos = startPos + maxDistance * forwardNormal;
						result.m_impactFraction = (newPos - startPos).GetLength() / maxDistance;
						result.m_impactDistance = (newPos - startPos).GetLength();
						return result;
					}
					prevBlockCoords.z = newBlockCoords.z;
					prevBlock = newBlock;
				}
			}
		}
	}

	DebugString(0.f, "[Raycast] Miss!", Rgba::RED, Rgba::RED);
	return result;
}

RaycastResult3D_t World::FastVoxelRaycast(const Vector3& startPos, const Vector3& forwardNormal, float maxDistance) const {
	float maxLengthSquared = maxDistance * maxDistance;

	//--------------------------------------------------------------------
	// Check if starting block is solid
	IntVector3 startBlockCoords((int)floorf(startPos.x), (int)floorf(startPos.y), (int)floorf(startPos.z));
	BlockLocator startBlockLocator;
	bool isSolid = IsBlockSolid(startBlockCoords, startBlockLocator);
	if (isSolid) {
		RaycastResult3D_t result;
		result.m_impactPos = startPos + maxDistance * forwardNormal;
		result.m_startPos = startPos;
		result.m_direction = forwardNormal;
		result.m_maxDistance = maxDistance;
		result.m_endPos = startPos + maxDistance * forwardNormal;
		result.m_impactFraction = 1.f;
		result.m_impactDistance = maxDistance;

		DebugString(0.f, "[Raycast] Start in solid!", Rgba::RED, Rgba::RED);
		return result;
	}

	//--------------------------------------------------------------------
	// Initialization
	Vector3 rayDisplacement = forwardNormal * maxDistance;

	float xDeltaT = rayDisplacement.x != 0.f ? 1.f / Abs(rayDisplacement.x) : 99999.f;
	float yDeltaT = rayDisplacement.y != 0.f ? 1.f / Abs(rayDisplacement.y) : 99999.f;
	float zDeltaT = rayDisplacement.z != 0.f ? 1.f / Abs(rayDisplacement.z) : 99999.f;

	int stepX = rayDisplacement.x > 0.f ? 1 : -1;
	int stepY = rayDisplacement.y > 0.f ? 1 : -1;
	int stepZ = rayDisplacement.z > 0.f ? 1 : -1;

	int offsetToLeadingEdgeX = (stepX + 1) / 2;
	int offsetToLeadingEdgeY = (stepY + 1) / 2;
	int offsetToLeadingEdgeZ = (stepZ + 1) / 2;

	float firstIntersectionX = (float)(startBlockCoords.x + offsetToLeadingEdgeX);
	float firstIntersectionY = (float)(startBlockCoords.y + offsetToLeadingEdgeY);
	float firstIntersectionZ = (float)(startBlockCoords.z + offsetToLeadingEdgeZ);

	float tOfNextXCrossing = Abs(firstIntersectionX - startPos.x) * xDeltaT;
	float tOfNextYCrossing = Abs(firstIntersectionY - startPos.y) * yDeltaT;
	float tOfNextZCrossing = Abs(firstIntersectionZ - startPos.z) * zDeltaT;

	IntVector3 blockCoords = startBlockCoords;
	float t;
	//--------------------------------------------------------------------
	// Main Raycast Loop
	
	while (true) {
		t = std::min(tOfNextXCrossing, std::min(tOfNextYCrossing, tOfNextZCrossing));

		if (t > 1) {
			// No impact, return to default
			RaycastResult3D_t result;
			result.m_impactPos = startPos + maxDistance * forwardNormal;
			result.m_startPos = startPos;
			result.m_direction = forwardNormal;
			result.m_maxDistance = maxDistance;
			result.m_endPos = startPos + maxDistance * forwardNormal;
			result.m_impactFraction = 1.f;
			result.m_impactDistance = maxDistance;
			return result;
		}

		//--------------------------------------------------------------------
		if (t == tOfNextXCrossing) {
			blockCoords.x += stepX;
			BlockLocator bl;
			if (IsBlockSolid(blockCoords, bl)) {
				// Impact!
				RaycastResult3D_t result;
				result.m_impactPos = startPos + tOfNextXCrossing * rayDisplacement;
				result.m_startPos = startPos;
				result.m_direction = forwardNormal;
				result.m_maxDistance = maxDistance;
				result.m_endPos = startPos + maxDistance * forwardNormal;
				result.m_impactFraction = tOfNextXCrossing;
				result.m_impactDistance = tOfNextXCrossing * maxDistance;
				result.m_impactBlock = bl;
				result.m_impactNormal = Vector3(-stepX, 0, 0);
				return result;
			}
			else {
				tOfNextXCrossing += xDeltaT;
			}
		}
		else if (t == tOfNextYCrossing) {
			blockCoords.y += stepY;
			BlockLocator bl;
			if (IsBlockSolid(blockCoords, bl)) {
				// Impact!
				RaycastResult3D_t result;
				result.m_impactPos = startPos + tOfNextYCrossing * rayDisplacement;
				result.m_startPos = startPos;
				result.m_direction = forwardNormal;
				result.m_maxDistance = maxDistance;
				result.m_endPos = startPos + maxDistance * forwardNormal;
				result.m_impactFraction = tOfNextYCrossing;
				result.m_impactDistance = tOfNextYCrossing * maxDistance;
				result.m_impactBlock = bl;
				result.m_impactNormal = Vector3(0, -stepY, 0);
				return result;
			}
			else {
				tOfNextYCrossing += yDeltaT;
			}
		}
		else if (t == tOfNextZCrossing) {
			blockCoords.z += stepZ;
			BlockLocator bl;
			if (IsBlockSolid(blockCoords, bl)) {
				// Impact!
				RaycastResult3D_t result;
				result.m_impactPos = startPos + tOfNextZCrossing * rayDisplacement;
				result.m_startPos = startPos;
				result.m_direction = forwardNormal;
				result.m_maxDistance = maxDistance;
				result.m_endPos = startPos + maxDistance * forwardNormal;
				result.m_impactFraction = tOfNextZCrossing;
				result.m_impactDistance = tOfNextZCrossing * maxDistance;
				result.m_impactBlock = bl;
				result.m_impactNormal = Vector3(0, 0, -stepZ);
				return result;
			}
			else {
				tOfNextZCrossing += zDeltaT;
			}
		}
	}
}

bool World::IsBlockSolid(const IntVector3& blockCoordsInWorld, BlockLocator& outBL) const {
	IntVector2 chunkCoords;
	if (blockCoordsInWorld.x < 0) {
		chunkCoords.x = ((blockCoordsInWorld.x + 1) / CHUNK_SIZE_X) - 1;	
	}
	else {
		chunkCoords.x = blockCoordsInWorld.x / CHUNK_SIZE_X;
	}

	if (blockCoordsInWorld.y < 0) {
		chunkCoords.y = ((blockCoordsInWorld.y + 1) / CHUNK_SIZE_Y) - 1;
	}
	else {
		chunkCoords.y = blockCoordsInWorld.y / CHUNK_SIZE_Y;
	}

	IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);


	if (m_activeChunks.find(chunkCoords) == m_activeChunks.end()) {
		// if chunk doesn't exist (raycast if out of range), return default miss
		return false;
	}
	else {
		Chunk* chunk = m_activeChunks.at(chunkCoords).get();
		int blockIndexInChunk = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);
		Block* b = &chunk->m_blocks[blockIndexInChunk];
		outBL = BlockLocator(chunk, blockIndexInChunk);
		bool isSolid = b->IsSolid();
		if (isSolid) {
			return true;
		}
		else {
			return false;
		}
	}
}

void World::Dig() {
	if (m_raycastResult.DidImpact()) {
		BlockLocator selectBlock = m_raycastResult.m_impactBlock;
		selectBlock.SetType(BLOCK_TYPE_AIR);
		selectBlock.m_chunk->m_isMeshDirty = true;
		selectBlock.m_chunk->m_needsSaving = true;

		if (selectBlock.GetBlockLocatorToEast().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToEast().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToWest().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToWest().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToNorth().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToNorth().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToSouth().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToSouth().m_chunk->m_isMeshDirty = true;
		}

		//--------------------------------------------------------------------
		// Set its new block type, and mark its light as dirty.  
		// Do NOT update its light exposures; let the lighting algorithm handle that (so it can cause chain reactions if necessary).
		MarkBlockLightingDirty(selectBlock);

		//--------------------------------------------------------------------
		// If the block ABOVE is flagged SKY, descend downward starting here until you reach the first opaque block directly below it, 
		// flagging each non - opaque block descended as SKY and dirtying it.
		BlockLocator bl = selectBlock;
		bl = bl.GetBlockLocatorAbove();
		Block* b = &bl.m_chunk->m_blocks[bl.m_blockIndex];
		if (b->IsBlockSky()) {
			bl = bl.GetBlockLocatorBelow();
			b = &bl.m_chunk->m_blocks[bl.m_blockIndex];
			while (!b->IsFullOpaque()) {
				b->SetIsBlockSky();
				//MarkBlockLightingDirty(bl);
				bl = bl.GetBlockLocatorBelow();
				b = &bl.m_chunk->m_blocks[bl.m_blockIndex];
			}
		}
	}
}

void World::Place(eBLockType blockType) {
	if (m_raycastResult.DidImpact() && m_raycastResult.m_impactDistance >= 2.f) {
		BlockLocator selectBlock = m_raycastResult.m_impactBlock;
		if (m_raycastResult.m_impactNormal == Vector3(1.f, 0.f, 0.f)) {
			selectBlock.StepEast();
		}
		else if (m_raycastResult.m_impactNormal == Vector3(-1.f, 0.f, 0.f)) {
			selectBlock.StepWest();
		}
		else if (m_raycastResult.m_impactNormal == Vector3(0.f, 1.f, 0.f)) {
			selectBlock.StepNorth();
		}
		else if (m_raycastResult.m_impactNormal == Vector3(0.f, -1.f, 0.f)) {
			selectBlock.StepSouth();
		}
		else if (m_raycastResult.m_impactNormal == Vector3(0.f, 0.f, 1.f)) {
			selectBlock.StepAbove();
		}
		else if (m_raycastResult.m_impactNormal == Vector3(0.f, 0.f, -1.f)) {
			selectBlock.StepBelow();
		}

		Block* b = &selectBlock.m_chunk->m_blocks[selectBlock.m_blockIndex];
		bool isOldBlockSky = b->IsBlockSky();

		selectBlock.SetType(blockType);
		selectBlock.m_chunk->m_isMeshDirty = true;
		selectBlock.m_chunk->m_needsSaving = true;

		bool isNewBlockOpaque = b->IsFullOpaque();

		MarkBlockLightingDirty(selectBlock);

		if (isOldBlockSky && isNewBlockOpaque) {
			b->ClearIsBlockSky();
			BlockLocator bl = selectBlock.GetBlockLocatorBelow();
			b = &bl.m_chunk->m_blocks[bl.m_blockIndex];
			while (!b->IsFullOpaque()) {
				b->ClearIsBlockSky();
				//MarkBlockLightingDirty(bl);
				bl = bl.GetBlockLocatorBelow();
				b = &bl.m_chunk->m_blocks[bl.m_blockIndex];
			}
				
		}

		if (selectBlock.GetBlockLocatorToEast().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToEast().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToWest().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToWest().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToNorth().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToNorth().m_chunk->m_isMeshDirty = true;
		}
		if (selectBlock.GetBlockLocatorToSouth().m_chunk != selectBlock.m_chunk) {
			selectBlock.GetBlockLocatorToSouth().m_chunk->m_isMeshDirty = true;
		}
	}
}

void World::RenderDebugPointsForDirtyLightingBlocks() const {
	if (!m_lightDirtyBlocks.empty()) {
		g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("debug"));

		Mesh<VertexPCU> points(PRIMITIVE_TYPE_POINTLIST, false);
		for (auto& b : m_lightDirtyBlocks) {
			IntVector3 blockCoords = b.m_chunk->GetBlockCoordsForBlockIndex(b.m_blockIndex);
			blockCoords.x += b.m_chunk->m_chunkCoords.x * CHUNK_SIZE_X;
			blockCoords.y += b.m_chunk->m_chunkCoords.y * CHUNK_SIZE_Y;
			points.AddPoint3D(Vector3(blockCoords) + Vector3(0.5f, 0.5f, 0.5f), Rgba::YELLOW);
		}
		g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&points);
	}
}

void World::UpdateDirtyLighting() {
	if (g_theApp->m_isDebugMode) {
// 		if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_L)) {
// 			int currentBlockCountsInQueue = m_lightDirtyBlocks.size();
// 			for (int i = 0; i < currentBlockCountsInQueue; ++i) {
// 				ProcessDirtyLightingBLock();
// 			}
// 		}
		while (!m_lightDirtyBlocks.empty()) {
			ProcessDirtyLightingBLock();
		}
	}
	else {
		while (!m_lightDirtyBlocks.empty()) {
			ProcessDirtyLightingBLock();
		}
	}
}

void World::ProcessDirtyLightingBLock() {
	BlockLocator bl = m_lightDirtyBlocks.front();
	//--------------------------------------------------------------------
	// Clear dirty light flag, so that it's no longer in the queue as well
	m_lightDirtyBlocks.pop_front();
	Block& b = bl.m_chunk->m_blocks[bl.m_blockIndex];
	b.ClearIsLightDirty();

	eBLockType type = bl.m_chunk->GetBlockType(bl.m_blockIndex);
	u8 indoorDefLightingLevel = BlockDefinition::GetDefinition(type)->m_indoorLightLevel;
	u8 indoorLightingLevel = b.GetIndoorLightLevel();
	u8 outdoorLightingLevel = b.GetOutdoorLightLevel();
	u8 desiredIndoorLightingLevel = 0U;
	u8 maxNeighborIndoorLightingLevel = 0U;
	u8 desiredOutdoorLightingLevel = outdoorLightingLevel;
	u8 maxNeighborOutdoorLightingLevel = 0U;
	bool isLightingCorrect = true;

	if (b.IsBlockSky()) {
		desiredOutdoorLightingLevel = 15U;
		if (outdoorLightingLevel != 15U) {
			isLightingCorrect = false;
		}
	}

	//--------------------------------------------------------------------
	// Check neighbors
	BlockLocator eastBL = bl.GetBlockLocatorToEast();
	if (eastBL.m_chunk) {
		Block& eastBlock = eastBL.m_chunk->m_blocks[eastBL.m_blockIndex];
		u8 eastIndoorLightingLevel = eastBlock.GetIndoorLightLevel();
		if (eastIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = eastIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 eastOutdoorLightingLevel = eastBlock.GetOutdoorLightLevel();
			if (eastOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = eastOutdoorLightingLevel;
			}
		}
	}

	BlockLocator westBL = bl.GetBlockLocatorToWest();
	if (westBL.m_chunk) {
		Block& westBlock = westBL.m_chunk->m_blocks[westBL.m_blockIndex];
		u8 westIndoorLightingLevel = westBlock.GetIndoorLightLevel();
		if (westIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = westIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 westOutdoorLightingLevel = westBlock.GetOutdoorLightLevel();
			if (westOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = westOutdoorLightingLevel;
			}
		}
	}

	BlockLocator northBL = bl.GetBlockLocatorToNorth();
	if (northBL.m_chunk) {
		Block& northBlock = northBL.m_chunk->m_blocks[northBL.m_blockIndex];
		u8 northIndoorLightingLevel = northBlock.GetIndoorLightLevel();
		if (northIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = northIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 northOutdoorLightingLevel = northBlock.GetOutdoorLightLevel();
			if (northOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = northOutdoorLightingLevel;
			}
		}
	}
	
	BlockLocator southBL = bl.GetBlockLocatorToSouth();
	if (southBL.m_chunk) {
		Block& southBlock = southBL.m_chunk->m_blocks[southBL.m_blockIndex];
		u8 southIndoorLightingLevel = southBlock.GetIndoorLightLevel();
		if (southIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = southIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 southOutdoorLightingLevel = southBlock.GetOutdoorLightLevel();
			if (southOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = southOutdoorLightingLevel;
			}
		}
	}

	BlockLocator aboveBL = bl.GetBlockLocatorAbove();
	if (aboveBL.m_chunk) {
		Block& aboveBlock = aboveBL.m_chunk->m_blocks[aboveBL.m_blockIndex];
		u8 aboveIndoorLightingLevel = aboveBlock.GetIndoorLightLevel();
		if (aboveIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = aboveIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 aboveOutdoorLightingLevel = aboveBlock.GetOutdoorLightLevel();
			if (aboveOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = aboveOutdoorLightingLevel;
			}
		}
	}

	BlockLocator belowBL = bl.GetBlockLocatorBelow();
	if (belowBL.m_chunk) {
		Block& belowBlock = belowBL.m_chunk->m_blocks[belowBL.m_blockIndex];
		u8 belowIndoorLightingLevel = belowBlock.GetIndoorLightLevel();
		if (belowIndoorLightingLevel > maxNeighborIndoorLightingLevel) {
			maxNeighborIndoorLightingLevel = belowIndoorLightingLevel;
		}
		if (!b.IsBlockSky()) {
			u8 belowOutdoorLightingLevel = belowBlock.GetOutdoorLightLevel();
			if (belowOutdoorLightingLevel > maxNeighborOutdoorLightingLevel) {
				maxNeighborOutdoorLightingLevel = belowOutdoorLightingLevel;
			}
		}
	}
	
	if (b.IsFullOpaque()) {
		desiredOutdoorLightingLevel = 0U;
		desiredIndoorLightingLevel = indoorDefLightingLevel;
		isLightingCorrect = false;
	}
	else {
		if (maxNeighborIndoorLightingLevel == 0U) {
			desiredIndoorLightingLevel = std::max((u8)0, indoorDefLightingLevel);
		}
		else {
			desiredIndoorLightingLevel = std::max((u8)(maxNeighborIndoorLightingLevel - 1U), indoorDefLightingLevel);
		}

		if (indoorLightingLevel != desiredIndoorLightingLevel) {
			isLightingCorrect = false;
		}

		if (maxNeighborOutdoorLightingLevel == 0U) {
			if (!b.IsBlockSky() && outdoorLightingLevel != 0U) {
				desiredOutdoorLightingLevel = 0U;
				isLightingCorrect = false;
			}
		}
		else if (maxNeighborOutdoorLightingLevel > 0U) {
			if (!b.IsBlockSky() && outdoorLightingLevel != (maxNeighborOutdoorLightingLevel - 1U)) {
				desiredOutdoorLightingLevel = maxNeighborOutdoorLightingLevel - 1U;
				isLightingCorrect = false;
			}
		}
	}

	//--------------------------------------------------------------------
	// If any of my lighting is incorrect
	// update my lighting level
	// Mark each of my 6 neighbors as dirty light, only if they are non opaque
	if(!isLightingCorrect) {
		// Finally recompute my lighting level;
		b.SetOutDoorLightLevel(desiredOutdoorLightingLevel);
		b.SetIndoorLightLevel(desiredIndoorLightingLevel);
		bl.m_chunk->m_isMeshDirty = true;

		if (eastBL.m_chunk) {
			eastBL.m_chunk->m_isMeshDirty = true;
			Block& eastBlock = eastBL.m_chunk->m_blocks[eastBL.m_blockIndex];
			if (!eastBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(eastBL);
			}
		}
		if (westBL.m_chunk) {
			westBL.m_chunk->m_isMeshDirty = true;
			Block& westBlock = westBL.m_chunk->m_blocks[westBL.m_blockIndex];
			if (!westBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(westBL);
			}
		}
		if (northBL.m_chunk) {
			northBL.m_chunk->m_isMeshDirty = true;
			Block& northBlock = northBL.m_chunk->m_blocks[northBL.m_blockIndex];
			if (!northBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(northBL);
			}
		}
		if (southBL.m_chunk) {
			southBL.m_chunk->m_isMeshDirty = true;
			Block& southBlock = southBL.m_chunk->m_blocks[southBL.m_blockIndex];
			if (!southBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(southBL);
			}
		}
		
		if (aboveBL.m_chunk) {
			Block& aboveBlock = aboveBL.m_chunk->m_blocks[aboveBL.m_blockIndex];
			if (!aboveBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(aboveBL);
			}
		}
		
		if (belowBL.m_chunk) {
			Block& belowBlock = belowBL.m_chunk->m_blocks[belowBL.m_blockIndex];
			if (!belowBlock.IsFullOpaque()) {
				MarkBlockLightingDirty(belowBL);
			}
		}

	}
}

void World::MarkBlockLightingDirty(BlockLocator& bl) {
	Block& b = bl.m_chunk->m_blocks[bl.m_blockIndex];
	if (!b.IsLightDirty()) {
		b.SetIsLightDirty();
		m_lightDirtyBlocks.emplace_back(bl);
	}
	// else do nothing
}

void World::UpdateWorldTime(float deltaSeconds) {
	float worldDeltaSec = deltaSeconds * m_timeScale;
	m_worldTime += worldDeltaSec / (60.f * 60.f * 24.f);
	
	float timeOfDay = Fract(m_worldTime);
	if (0.25f <= timeOfDay && timeOfDay <= 0.75f) {
		if (timeOfDay < 0.5f) {
			float fraction = RangeMapFloat(timeOfDay, 0.25f, 0.5f, 0.f, 1.f);
			m_skyColor = Interpolate(Rgba(20, 20, 24), Rgba(200, 230, 255), fraction);
		}
		else {
			float fraction = RangeMapFloat(timeOfDay, 0.5f, 0.75f, 0.f, 1.f);
			m_skyColor = Interpolate(Rgba(200, 230, 255), Rgba(20, 20, 24), fraction);
		}
	}
	else {
		m_skyColor = Rgba(20, 20, 24);
	}
	g_theRHI->GetImmediateRenderer()->SetSkyColor(m_skyColor);
}

void World::ApplyLightningStrikeToSkyColor() {
	float noiseValue = Compute1dPerlinNoise(m_worldTime * (60.f * 60.f * 24.f), 100.f, 9);
	m_lightningStrength = RangeMapFloat(noiseValue, 0.6f, 0.9f, 0.f, 1.f);
	m_lightningStrength = Clamp01(m_lightningStrength);
	m_skyColor = Interpolate(m_skyColor, Rgba::WHITE, m_lightningStrength);
	g_theRHI->GetImmediateRenderer()->SetSkyColor(m_skyColor);
}

void World::ApplyGlowstoneFlickerToIndoorLight() {
	float noiseValue = Compute1dPerlinNoise(m_worldTime * (60.f * 60.f * 24.f), 100.f, 9);
	m_glowStrength = RangeMapFloat(noiseValue, -1.f, 1.f, 0.8f, 1.f);
	Rgba newIndoorColor = m_baseIndoorColor;
	newIndoorColor.ScaleRGB(m_glowStrength);
	g_theRHI->GetImmediateRenderer()->SetIndoorColor(newIndoorColor);
}

void World::ApplyNewtonianPhysics(float ds) {
	// Apply gravity in Z direction
	m_player->m_acceleration.z = -9.8f;
	DebugString(0.f, Stringf("Acceleration: %.2f, %.2f, %.2f", m_player->m_acceleration.x, m_player->m_acceleration.y, m_player->m_acceleration.z), Rgba::CYAN, Rgba::CYAN);

	// Apply friction
	if (m_player->m_isOnGround) {
		float u = 0.01f;
		float N = 1.f;
		Vector3 v = m_player->m_velocity.GetNormalized();
		m_player->m_acceleration += -1.f * u * N * v;
	}

	// Apply air drag force
	{
		float c = 0.1f;
		float squaredV = m_player->m_velocity.GetLengthSquared();
		Vector3 v = m_player->m_velocity.GetNormalized();
		m_player->m_acceleration += -1.f * squaredV * c * v;
	}


	// Add velocity
	m_player->m_velocity += m_player->m_acceleration * ds;
	DebugString(0.f, Stringf("Speed: %.2f", m_player->m_velocity.GetLength()), Rgba::CYAN, Rgba::CYAN);


	// Translate player
	Vector3 velocityInEngine(-m_player->m_velocity.y, m_player->m_velocity.z, m_player->m_velocity.x);
	m_player->m_transform.Translate(velocityInEngine * ds);

	// Debug velocity
	Vector3 playerPositionInEngine = m_player->m_transform.GetWorldPosition();
	Vector3 playerPosition(playerPositionInEngine.z, -playerPositionInEngine.x, playerPositionInEngine.y);
	DebugDrawLine3D(0.f, playerPosition, playerPosition + m_player->m_velocity, Rgba::YELLOW, Rgba::YELLOW);
}

void World::ApplyCorrectiveCollisionDetection() {
	Vector3 playerPositionInEngine = m_player->m_transform.GetWorldPosition();
	Vector3 playerPosition(playerPositionInEngine.z, -playerPositionInEngine.x, playerPositionInEngine.y);
	IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(playerPosition);
	IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);

	DebugString(0.f, Stringf("BlockCoordsInChunk %d, %d, %d", blockCoordsInChunk.x, blockCoordsInChunk.y, blockCoordsInChunk.z), Rgba::YELLOW, Rgba::YELLOW);

	if (m_player->m_colliderType == COLLIDER_TYPE_UNIBALL) {
		BlockLocator startBlockLocator;
		Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
		if (chunk) {
			int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);
			startBlockLocator = BlockLocator(chunk, blockIndex);
		}
		else {
			return;
		}

		{
			BlockLocator eastBL = startBlockLocator.GetBlockLocatorToEast();
			if (eastBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(1, 0, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.x -= (0.5f - distance);
					m_player->m_velocity.x = 0.f;
				}
			}
			BlockLocator westBL = startBlockLocator.GetBlockLocatorToWest();
			if (westBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(-1, 0, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.x += (0.5f - distance);
					m_player->m_velocity.x = 0.f;
				}
			}
			BlockLocator northBL = startBlockLocator.GetBlockLocatorToNorth();
			if (northBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, 1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.y -= (0.5f - distance);
					m_player->m_velocity.y = 0.f;
				}
			}
			BlockLocator southBL = startBlockLocator.GetBlockLocatorToSouth();
			if (southBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, -1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.y += (0.5f - distance);
					m_player->m_velocity.y = 0.f;
				}
			}
 			BlockLocator aboveBL = startBlockLocator.GetBlockLocatorAbove();
			if (aboveBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, 0, 1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.z -= (0.5f - distance);
					m_player->m_velocity.z = 0.f;
				}
			}
			BlockLocator belowBL = startBlockLocator.GetBlockLocatorBelow();
			if (belowBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, 0, -1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition.z += (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator belowWestBL = belowBL.GetBlockLocatorToWest();
			if (belowWestBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(-1, 0, -1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator belowEastBL = belowBL.GetBlockLocatorToEast();
			if (belowEastBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(1, 0, -1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator belowNorthBL = belowBL.GetBlockLocatorToNorth();
			if (belowNorthBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, 1, -1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator belowSouthBL = belowBL.GetBlockLocatorToSouth();
			if (belowSouthBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(0, -1, -1);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}

			BlockLocator northWestBL = northBL.GetBlockLocatorToWest();
			if (northWestBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(-1, 1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator northEastBL = northBL.GetBlockLocatorToEast();
			if (northEastBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(1, 1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator southWestBL = southBL.GetBlockLocatorToWest();
			if (southWestBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(-1, -1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
			BlockLocator southEastBL = southBL.GetBlockLocatorToEast();
			if (southEastBL.IsSolid()) {
				IntVector3 newBlockCoordsInWorld = blockCoordsInWorld + IntVector3(1, -1, 0);
				Vector3 closestPoint = GetClosestPointOnBlock(newBlockCoordsInWorld, playerPosition);
				float distance = (playerPosition - closestPoint).GetLength();
				if (distance < 0.5f) {
					playerPosition += (playerPosition - closestPoint).GetNormalized() * (0.5f - distance);
					m_player->m_velocity.z = 0.f;
					m_player->m_isOnGround = true;
				}
			}
		}

		Vector3 newPositionInEngine(-playerPosition.y, playerPosition.z, playerPosition.x);
		m_player->m_transform.SetWorldPosition(newPositionInEngine);
	}
	else if (m_player->m_colliderType == COLLIDER_TYPE_AABB3) {
	
		int iteration = 3;
		while (iteration--) {

			float suggestX = 9999.f;
			float suggestY = 9999.f;
			float suggestZ = 9999.f;

			Vector3 blfCorner = playerPosition + Vector3(-0.3f, -0.3f, 0.f);
			Vector3 brfCorner = playerPosition + Vector3(0.3f, -0.3f, 0.f);
			Vector3 blbCorner = playerPosition + Vector3(-0.3f, 0.3f, 0.f);
			Vector3 brbCorner = playerPosition + Vector3(0.3f, 0.3f, 0.f);
			Vector3 tlfCorner = playerPosition + Vector3(-0.3f, -0.3f, 1.8f);
			Vector3 trfCorner = playerPosition + Vector3(0.3f, -0.3f, 1.8f);
			Vector3 tlbCorner = playerPosition + Vector3(-0.3f, 0.3f, 1.8f);
			Vector3 trbCorner = playerPosition + Vector3(0.3f, 0.3f, 1.8f);

			//blfCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(blfCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator aboveBL = thisBL.GetBlockLocatorAbove();
						if (!aboveBL.IsSolid() && aboveBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = aboveBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(aboveBL.m_blockIndex);
							dispZ = (float)newBlockCoordsInWorld.z - blfCorner.z + 0.001f;
						}

						BlockLocator eastBL = thisBL.GetBlockLocatorToEast();
						if (!eastBL.IsSolid() && eastBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = eastBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(eastBL.m_blockIndex);
							dispX = (float)newBlockCoordsInWorld.x - blfCorner.x + 0.01f;
						}

						BlockLocator northBL = thisBL.GetBlockLocatorToNorth();
						if (!northBL.IsSolid() && northBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = northBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(northBL.m_blockIndex);
							dispY = (float)newBlockCoordsInWorld.y - blfCorner.y + 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}
						
					}
				}
			}

			//brfCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(brfCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator aboveBL = thisBL.GetBlockLocatorAbove();
						if (!aboveBL.IsSolid() && aboveBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = aboveBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(aboveBL.m_blockIndex);
							dispZ = (float)newBlockCoordsInWorld.z - brfCorner.z + 0.01f;
						}

						BlockLocator westBL = thisBL.GetBlockLocatorToWest();
						if (!westBL.IsSolid() && westBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = westBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(westBL.m_blockIndex);
							dispX = (float)(newBlockCoordsInWorld.x + 1) - brfCorner.x - 0.01f;
						}

						BlockLocator northBL = thisBL.GetBlockLocatorToNorth();
						if (!northBL.IsSolid() && northBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = northBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(northBL.m_blockIndex);
							dispY = (float)newBlockCoordsInWorld.y - brfCorner.y;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}
					}
				}
			}

			//blbCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(blbCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator aboveBL = thisBL.GetBlockLocatorAbove();
						if (!aboveBL.IsSolid() && aboveBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = aboveBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(aboveBL.m_blockIndex);
							dispZ = (float)newBlockCoordsInWorld.z - blbCorner.z + 0.001f;
						}

						BlockLocator eastBL = thisBL.GetBlockLocatorToEast();
						if (!eastBL.IsSolid() && eastBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = eastBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(eastBL.m_blockIndex);
							dispX = (float)newBlockCoordsInWorld.x - blbCorner.x + 0.01f;
						}

						BlockLocator southBL = thisBL.GetBlockLocatorToSouth();
						if (!southBL.IsSolid() && southBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = southBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(southBL.m_blockIndex);
							dispY = (float)(newBlockCoordsInWorld.y + 1) - blbCorner.y - 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}

					}
				}
			}

			//brbCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(brbCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator aboveBL = thisBL.GetBlockLocatorAbove();
						if (!aboveBL.IsSolid() && aboveBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = aboveBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(aboveBL.m_blockIndex);
							dispZ = (float)newBlockCoordsInWorld.z - brbCorner.z + 0.001f;
						}

						BlockLocator westBL = thisBL.GetBlockLocatorToWest();
						if (!westBL.IsSolid() && westBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = westBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(westBL.m_blockIndex);
							dispX = (float)(newBlockCoordsInWorld.x + 1) - brbCorner.x - 0.01f;
						}

						BlockLocator southBL = thisBL.GetBlockLocatorToSouth();
						if (!southBL.IsSolid() && southBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = southBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(southBL.m_blockIndex);
							dispY = (float)(newBlockCoordsInWorld.y + 1) - brbCorner.y - 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}
					}
				}
			}

			//tlfCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(tlfCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator belowBL = thisBL.GetBlockLocatorBelow();
						if (!belowBL.IsSolid() && belowBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = belowBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(belowBL.m_blockIndex);
							dispZ = (float)(newBlockCoordsInWorld.z + 1) - tlfCorner.z - 0.001f;
						}

						BlockLocator eastBL = thisBL.GetBlockLocatorToEast();
						if (!eastBL.IsSolid() && eastBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = eastBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(eastBL.m_blockIndex);
							dispX = (float)newBlockCoordsInWorld.x - tlfCorner.x + 0.01f;
						}

						BlockLocator northBL = thisBL.GetBlockLocatorToNorth();
						if (!northBL.IsSolid() && northBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = northBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(northBL.m_blockIndex);
							dispY = (float)newBlockCoordsInWorld.y - tlfCorner.y + 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}

					}
				}
			}

			//trfCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(trfCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator belowBL = thisBL.GetBlockLocatorBelow();
						if (!belowBL.IsSolid() && belowBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = belowBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(belowBL.m_blockIndex);
							dispZ = (float)(newBlockCoordsInWorld.z + 1) - trfCorner.z - 0.001f;
						}

						BlockLocator westBL = thisBL.GetBlockLocatorToWest();
						if (!westBL.IsSolid() && westBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = westBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(westBL.m_blockIndex);
							dispX = (float)(newBlockCoordsInWorld.x + 1) - trfCorner.x - 0.01f;
						}

						BlockLocator northBL = thisBL.GetBlockLocatorToNorth();
						if (!northBL.IsSolid() && northBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = northBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(northBL.m_blockIndex);
							dispY = (float)newBlockCoordsInWorld.y - trfCorner.y + 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}
					}
				}
			}

			//tlbCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(tlbCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator belowBL = thisBL.GetBlockLocatorBelow();
						if (!belowBL.IsSolid() && belowBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = belowBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(belowBL.m_blockIndex);
							dispZ = (float)(newBlockCoordsInWorld.z + 1) - tlbCorner.z - 0.001f;
						}

						BlockLocator eastBL = thisBL.GetBlockLocatorToEast();
						if (!eastBL.IsSolid() && eastBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = eastBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(eastBL.m_blockIndex);
							dispX = (float)newBlockCoordsInWorld.x - tlbCorner.x + 0.01f;
						}

						BlockLocator southBL = thisBL.GetBlockLocatorToSouth();
						if (!southBL.IsSolid() && southBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = southBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(southBL.m_blockIndex);
							dispY = (float)(newBlockCoordsInWorld.y + 1) - tlbCorner.y - 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}

					}
				}
			}

			//trbCorner
			{
				IntVector3 blockCoordsInWorld = GetBlockCoordsInWorld(trbCorner);
				IntVector3 blockCoordsInChunk = GetBlockCoordsInChunkFromWorld(blockCoordsInWorld);
				Chunk* chunk = GetChunkForBlockCoordsInWorld(blockCoordsInWorld);
				if (chunk) {
					int blockIndex = chunk->GetBlockIndexForBlockCoords(blockCoordsInChunk);

					BlockLocator thisBL(chunk, blockIndex);
					if (thisBL.IsSolid()) {
						float dispX = 0.f;
						float dispY = 0.f;
						float dispZ = 0.f;

						BlockLocator belowBL = thisBL.GetBlockLocatorBelow();
						if (!belowBL.IsSolid() && belowBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = belowBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(belowBL.m_blockIndex);
							dispZ = (float)(newBlockCoordsInWorld.z + 1) - trbCorner.z - 0.001f;
						}

						BlockLocator westBL = thisBL.GetBlockLocatorToWest();
						if (!westBL.IsSolid() && westBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = westBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(westBL.m_blockIndex);
							dispX = (float)(newBlockCoordsInWorld.x + 1) - trbCorner.x - 0.01f;
						}

						BlockLocator southBL = thisBL.GetBlockLocatorToSouth();
						if (!southBL.IsSolid() && southBL.m_chunk) {
							IntVector3 newBlockCoordsInWorld = southBL.m_chunk->GetBlockCoordsInWorldFromBlockIndexInChunk(southBL.m_blockIndex);
							dispY = (float)(newBlockCoordsInWorld.y + 1) - trbCorner.y - 0.01f;
						}

						if (dispX != 0.f) {
							if (Abs(dispX) < Abs(suggestX)) {
								suggestX = dispX;
							}
						}
						if (dispY != 0.f) {
							if (Abs(dispY) < Abs(suggestY)) {
								suggestY = dispY;
							}
						}
						if (dispZ != 0.f) {
							if (Abs(dispZ) < Abs(suggestZ)) {
								suggestZ = dispZ;
							}
						}
					}
				}
			}

			// Eliminate velocity
			if (suggestX != 9999.f && Abs(suggestX) == std::min(Abs(suggestX), std::min(Abs(suggestY), Abs(suggestZ)))) {
				m_player->m_velocity.x = 0.f;
				playerPosition += Vector3(suggestX, 0.f, 0.f);
			}
			else if (suggestY != 9999.f && Abs(suggestY) == std::min(Abs(suggestX), std::min(Abs(suggestY), Abs(suggestZ)))) {
				m_player->m_velocity.y = 0.f;
				playerPosition += Vector3(0.f, suggestY, 0.f);
			}
			else if (suggestZ != 9999.f && Abs(suggestZ) == std::min(Abs(suggestX), std::min(Abs(suggestY), Abs(suggestZ)))) {
				m_player->m_velocity.z = 0.f;
				m_player->m_isOnGround = true;
				playerPosition += Vector3(0.f, 0.f, suggestZ);
			}

			Vector3 newPositionInEngine(-playerPosition.y, playerPosition.z, playerPosition.x);
			m_player->m_transform.SetWorldPosition(newPositionInEngine);
		}
	}
}

Vector3 World::GetClosestPointOnBlock(const IntVector3& blockCoordsInWorld, const Vector3& position) const {
	Vector3 blockMin(blockCoordsInWorld);
	Vector3 blockMax = blockMin + Vector3(1, 1, 1);
	float x = std::max(blockMin.x, std::min(position.x, blockMax.x));
	float y = std::max(blockMin.y, std::min(position.y, blockMax.y));
	float z = std::max(blockMin.z, std::min(position.z, blockMax.z));

	return Vector3(x, y, z);
}
