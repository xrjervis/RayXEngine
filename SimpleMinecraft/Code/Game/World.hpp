#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Core/type.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockLocator.hpp"
#include <map>
#include <deque>

struct RaycastResult3D_t {
	Vector3			m_startPos = Vector3::ZERO;
	Vector3			m_direction = Vector3::ZERO;
	float			m_maxDistance = 0.f;
	Vector3			m_endPos = Vector3::ZERO;
	Vector3			m_impactPos = Vector3::ZERO;
	float			m_impactFraction = 0.f;
	float			m_impactDistance = 0.f;
	BlockLocator	m_impactBlock = BlockLocator(nullptr, 0);
	Vector3			m_impactNormal = Vector3::ZERO;

	bool DidImpact() const { return m_impactFraction < 1.f; }
};

class Camera;
class GameCamera;
class Player;


class World {
public:
	World();
	~World();

	void Update(float deltaSeconds);
	void Render() const;

private:
	Chunk*				ActivateChunk(const IntVector2& chunkCoords);
	void				DeactivateChunk(const IntVector2& chunkCoords);
	void				DeactivateAllChunks();
	void				RenderChunks() const;
	IntVector2			GetCameraCurrentChunkCoords() const;
	Vector3				GetCameraCurrentPosition() const;
	IntVector3			GetBlockCoordsInChunkFromWorld(const IntVector3& blockCoordsInWorld) const;
	IntVector3			GetBlockCoordsInWorld(const Vector3& position) const;
	Chunk*				GetChunkForBlockCoordsInWorld(const IntVector3& blockCoordsInWorld) const;
	void				FindAndActivateNearestChunkInRange();
	void				FindAndDeactivateFarthestChunkOutRange();
	void				FindAndRebuildChunkMesh() const;
	void				LinkUpChunkNeighbor(const IntVector2& chunkCoords) const;
	void				UpdateInput(float deltaSeconds);
	RaycastResult3D_t	StepAndSampleRaycast(const Vector3& startPos, const Vector3& forwardNormal, float maxDistance) const;
	RaycastResult3D_t	FastVoxelRaycast(const Vector3& startPos, const Vector3& forwardNormal, float maxDistance) const;
	bool				IsBlockSolid(const IntVector3& blockCoordsInWorld, BlockLocator& outBL) const;

	void				Dig();
	void				Place(eBLockType blockType);

	void				RenderDebugPointsForDirtyLightingBlocks() const;
	void				UpdateDirtyLighting();
	void				ProcessDirtyLightingBLock();
	void				MarkBlockLightingDirty(BlockLocator& bl);

	void				UpdateWorldTime(float deltaSeconds);
	void				ApplyLightningStrikeToSkyColor();
	void				ApplyGlowstoneFlickerToIndoorLight();

	void				ApplyNewtonianPhysics(float ds);
	void				ApplyCorrectiveCollisionDetection();
	Vector3				GetClosestPointOnBlock(const IntVector3& blockCoordsInWorld, const Vector3& position) const;


public:
	std::map<IntVector2, Uptr<Chunk>> m_activeChunks;

	Uptr<Camera>		m_mainCamera2D;
	Uptr<GameCamera>	m_mainCamera3D;

	RaycastResult3D_t	m_raycastResult;

	std::deque<BlockLocator> m_lightDirtyBlocks;

	Rgba m_skyColor;
	Rgba m_baseIndoorColor;
	Rgba m_baseOutdoorColor;
	float m_worldTime = 0.f;
	float m_timeScale = 200.f;
	float m_lightningStrength = 0.f;
	float m_glowStrength = 0.f;


	Uptr<Player> m_player;
};