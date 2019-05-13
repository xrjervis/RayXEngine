#pragma once
#include "Engine/Math/RaycastResult.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Game/Board.hpp"
#include "Game/Thief.hpp"
#include "Game/Guard.hpp"

class SpriteSheet;
class Blackboard;
class Camera;

constexpr int RAYCAST_STEPS_PER_Unit = 1;

class EscapeWorld {
public:
	EscapeWorld();
	~EscapeWorld();

	void InitBoard();
	void InitCameras();
	void LoadGameConfigData();
	void LoadDefaultResources();
	void LoadMap(const std::string& filePath);
	RaycastResult2D_t Raycast(const Vector2& startPos, const Vector2& direction, float maxDistance) const;
	RaycastResult2D_t RaycastOnThiefWindow(const Vector2& startPos, const Vector2& direction, float maxDistance) const;
	float GetDepthValueAtGridCoords(const IntVector2& gridCoords);
	void FindPatternInArea(const Pattern_t& pattern, const AABB2& bounds, std::vector<IntVector2>& out);
	bool IsPatternThere(const Pattern_t& pattern, const IntVector2& gridCoords);
	void FindMovableGridsWithinRange(const IntVector2& startCoords, int range, std::vector<IntVector2>& out);
	bool IsGuardThere(const IntVector2& gridCoords);
	bool CheckIfThiefIsInRoom(std::vector<IntVector2>& out);

	Vector2 GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const;
	Vector2 GetGridPositionFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const;
	Vector2 GetGridCenterFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const;
	Vector2 GetGridCenterFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const;

	std::string GetStringFromActionType(eActionType actionType) const;

	bool CheckIfMonitorCanSeeThief(const IntVector2& monitorGridCoords);

public:
	Uptr<Blackboard> m_gameConfig;
	Uptr<Camera> m_thiefCamera;
	Uptr<Camera> m_boardCamera;

	Uptr<Board> m_board;
	float m_gridWidthInThiefWindow;
	float m_gridHeightInThiefWindow;
	float m_widthInThiefWindow;
	float m_heightInThiefWindow;

	//--------------------------------------------------------------------
	const static Pattern_t s_monitorPattern;
	const static Pattern_t s_emptyPattern;
	const static Pattern_t s_flashLightPattern;
	const static Pattern_t s_singleMonitorPattern;
	const static Pattern_t s_scanAllMonitorsPattern;
	const static Pattern_t s_motionDetectorPattern;

	const static IntVector2 s_monitorSpriteCoords;
	const static IntVector2 s_unkownSpriteCoords;
	const static IntVector2 s_treasureSpriteCoords;
	const static IntVector2 s_treasureOpenSpriteCoords;
	const static IntVector2 s_guardCursorSpriteCoords;

	const static AABB2 s_playableAreaBounds;
	const static AABB2 s_infoAreaBounds;
	const static AABB2 s_actionAreaBounds;
	const static AABB2 s_actionItemSlotBounds;
	const static AABB2 s_equipmentSlotBounds;

	//--------------------------------------------------------------------
	// Map data
	std::array<Tile, GRID_COUNT_TOTAL>	m_tiles;
	SpriteSheet*						m_tileSet;

	// Entities
	Thief								m_thief;
	std::vector<Guard>					m_guards;
	int									m_currentGuardIndex = 0;

	//
	std::map<IntVector2, bool>			m_doorsAndWinodwsStates;
	int									m_totalMonitors;
	int									m_totalTreasures;
	int									m_leastTreasuresStolenToWin;
	int									m_monitorAlertDistanceDefault;
	std::map<IntVector2, bool>			m_monitors;
	std::map<IntVector2, bool>			m_monitorsGuardKnows;
	std::map<IntVector2, bool>			m_treasures;
	std::map<IntVector2, bool>			m_treasuresGuardKnows;
	std::vector<std::vector<IntVector2>> m_rooms;

	bool								m_doesThiefWin = false;
	int									m_totalTurns = 0;
};    