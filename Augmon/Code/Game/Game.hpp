// #pragma once
// #include "Engine/Core/type.hpp"
// #include "Engine/Math/AABB2.hpp"
// #include "Engine/Math/IntVector2.hpp"
// #include "Engine/Math/Vector2.hpp"
// #include "Game/GameState.hpp"
// #include "Game/Action.hpp"
// #include "Game/Board.hpp"
// #include <string>
// #include <vector>
// #include <deque>
// #include <map>
// #include <unordered_map>
// 
// class Clock;
// class Camera;
// class Manager;
// class Mesh;
// 
// 
// 
// class Game {
// public: 
// 	Game();
// 	~Game();
// 
// 	void Init();
// 	void Update();
// 	void Render() const;
// 
// private:
// 	Vector2 GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const;
// 	Vector2 GetGridPositionFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const;
// 	std::string ConvertGameStateToString(eGameState gameState) const;
// 	std::string ConvertActionTypeToString(eActionType actionType) const;
// 	void FindMovableGridsWithinRange(const IntVector2& startCoords, int range, std::vector<IntVector2>& out);
// 	void GenerateDoorsAndWinodwsOpenOrClose();
// 	void CheckIfGuardCanSeeThief();
// 	void CheckIsThiefOnTreasure();
// 	void CheckIsThiefOnCamera();
// 	void CheckIsGuardOnCamera();
// 	bool FindPatternInArea(const Pattern_t& pattern, const AABB2& bounds, IntVector2& out);
// 	void GenerateTreasuresOnBoard();
// 
// 	void ClearThiefWindow() const;
// 	void ClearBoardWindow() const;
// 
// 	void Update_Attract();
// 	void Render_Attract() const;
// 
// 	void Update_Camera_Setup();
// 	void Render_Camera_Setup() const;
// 
// 	void Update_Guard_Setup();
// 	void Render_Guard_Setup() const;
// 
// 	void Update_Thief_Setup();
// 	void Render_Thief_Setup() const;
// 
// 	void Update_Thief_Move();
// 	void Render_Thief_Move() const;
// 
// 	void Update_Guard_Wait();
// 	void Render_Guard_Wait() const;
// 
// 	void Update_Guard_Move();
// 	void Render_Guard_Move() const;
// 
// 	void Update_Guard_Act();
// 	void Render_Guard_Act() const;
// 
// 	void Update_Thief_Win();
// 	void Render_Thief_Win() const;
// 
// 	void Update_Guard_Win();
// 	void Render_Guard_Win() const;
// 
// 	void RenderDebugStuffInThiefWindow() const;
// 	void RenderGridLines() const;
// 	void RenderInfoAreaBackground() const;
// 	void RenderActionAreaBackground() const;
// 	void RenderMapOnBoardWindow() const;
// 	void RenderMapOnThiefWindow() const;
// 	void ShowAvailableTilesForGuardSetup() const;
// 
// 	void RebuildTileMeshOnThief();
// 	void RenderGuardsOnThiefWindow() const;
// 	void RenderWhiteTileForGuardOnBoard() const;
// 	void UpdateThiefCursor();
// 	void RenderThiefCursor() const;
// 	void RenderThiefMoveInInfoArea() const;
// 	void RenderThiefSetupInInfoArea() const;
// 	void RenderGuardSetupInInfoArea() const;
// 	void ShowAvailableTilesForThiefSetup() const;
// 	void ShowAvailableTilesForThiefMove() const;
// 	void RenderThiefOnThiefWindow() const;
// 	void RenderThiefOnBoardWindow() const;
// 	void ShowAvailableTilesForGuardMove() const;
// 
// 	bool CheckThiefWinCondition();
// 	bool CheckGuardWinCondition();
// 
// 	void RollDiceForGuards();
// 	void RenderActionAndMovementInInfoArea() const;
// 	void RenderBasicInActionArea() const;
// 	void RenderActionItemNameInActionArea() const;
// 	void RenderEquipmentNameInActionArea() const;
// 	void RenderFlashlightBatteryInActionArea() const;
// 
// 	void RenderAvailableTilesForCameraSetup() const;
// 	void RenderWhiteTileForCamerasOnBoard() const;
// 	void RenderCamerasOnBoard() const;
// 	void RenderCameraOnThief() const;
// 	void RenderCameraSetupInInfoArea() const;
// 	void ShowAvailableTilesForCameras() const;
// 	void ShowWhiteTilesForTheRoomThiefIsIn() const;
// 	void RenderTreasuresOnBoard() const;
// 	void RenderTreasuresOnThief() const;
// 
// 	void ShowNeedMoreTreasureText(float normalizedTime, const Vector2& pos);
// 	void ShowLockedText(float normalizedTime, const Vector2& pos);
// 	void ShowEquipFlashlight(float normalizedTime, const Vector2& pos);
// 	void ShowFlashlightBatteryIsLow(float normalizedTime, const Vector2& pos);
// 	void ShowCheckOneCamera(float normalizedTime, const Vector2& pos);
// 	void ShowScanAllCameras(float normalizedTime, const Vector2& pos);
// 	void ShowCameraCannotSeeThief(float normalizedTime, const Vector2& pos);
// 	void ShowCameraIsDisconnected(float normalizedTime, const Vector2& pos);
// 	void ShowCameraIsRecovered(float normalizedTime, const Vector2& pos);
// 	void ShowNoCameraAvailable(float normalizedTime, const Vector2& pos);
// 	void ShowPickUpTreasure(float normalizedTime, const Vector2& pos);
// 	void ShowDisconnectCamera(float normalizedTime, const Vector2& pos);
// 	void ShowTreasureStolen(float normalizedTime, const Vector2& pos);
// 	void ShowMotionDetector(float normalizedTime, const Vector2& pos);
// 
// 
// 
// public:
// 	//--------------------------------------------------------------------
// 	// Game constants
// 	int				m_thiefFixedMoveDistance = 3;
// 	int				m_totalCameras = 4;
// 	int				m_totalTreasures = 8;
// 	int				m_leastTreasuresStolenToWin = 3;
// 	int				m_guardFlashlightBatteryRemained = 10;
// 	int				m_guardFlashlightBatteryTotal = 10;
// 	int				m_thiefMaxMoveDistance = 3;
// 
// 	Uptr<Clock>		m_gameClock;
// 	Uptr<Camera>	m_thiefCamera;
// 	Uptr<Camera>	m_boardCamera;
// 
// 	eGameState		m_currentState;
// 	eGameState		m_nextState;
// 
// 	Uptr<Board>		m_board;
// 	Uptr<Mesh>		m_tileMesh;
// 
// 	float			m_gridWidthInThiefWindow;
// 	float			m_gridHeightInThiefWindow;
// 	AABB2			m_infoAreaBounds = AABB2(Vector2(0.f, 0.f), Vector2(21.f, 4.f));
// 	AABB2			m_actionAreaBounds = AABB2(Vector2(21.f, 0.f), Vector2(42.f, 4.f));
// 	AABB2			m_actionItemSlotBounds = AABB2(Vector2(24.f, 1.f), Vector2(26.f, 3.f));
// 	AABB2			m_equipmentSlotBounds = AABB2(Vector2(32.f, 1.f), Vector2(35.f, 3.f));
// 
// 	bool			m_isThiefOnBoard = false;
// 	bool			m_isThiefHidden = true;
// 	IntVector2		m_thiefCursorCoords;
// 	IntVector2		m_thiefGridCoords;
// 	std::vector<IntVector2> m_thiefAvailableMoveGrids;
// 	std::map<IntVector2, bool>	m_doorsAndWinodwsStates;
// 	int				m_treasuresStolenByThief = 0;
// 	bool		    m_isAnyTreasureStolenLastTurn = false;
// 	IntVector2		m_treasureStolenLastTurn;
// 	bool			m_isThiefInRoom;
// 	AABB2			m_theRoomThiefIsIn;
// 
// 	bool			m_isGuardOnBoard = false;
// 	IntVector2		m_guardGridCoords;
// 	std::vector<IntVector2> m_guardAvailableMoveGrids;
// 	std::vector<eActionType> m_guardActionsInHand = { ACTION_TYPE_SINGLECAMERA, ACTION_TYPE_SCANCAMERA, ACTION_TYPE_MOTIONDETECTOR };
// 	eActionType		m_actionOnDiceThisTurn;
// 	int				m_movementOnDiceThisTurn;
// 	bool			m_hasGuardTakenActionThisTurn;
// 	eActionType		m_guardActionItem = ACTION_TYPE_UNKNOWN;
// 	eActionType		m_guardEquipment = ACTION_TYPE_UNKNOWN;
// 	bool			m_guardUsedFlashlightThisTurn = false;
// 
// 
// 	std::map<IntVector2, bool> m_cameras;
// 	std::vector<IntVector2> m_camerasGuardKnows;
// 	
// 	std::map<IntVector2, bool> m_treasures;
// 
// 	std::vector<AABB2> m_rooms = { 
// 		AABB2(Vector2(13, 20), Vector2(29, 21)),
// 		AABB2(Vector2(9, 10), Vector2(11, 15)),
// 		AABB2(Vector2(13, 14), Vector2(15, 18)),
// 		AABB2(Vector2(26, 14), Vector2(28, 18)),
// 		AABB2(Vector2(19, 11), Vector2(22, 13)),
// 		AABB2(Vector2(13, 8), Vector2(15, 10)),
// 		AABB2(Vector2(26, 9), Vector2(28, 12)),
// 		AABB2(Vector2(17, 5), Vector2(18, 6)),
// 		AABB2(Vector2(23, 5), Vector2(24, 6)),
// 		AABB2(Vector2(26, 6), Vector2(30, 7))
// 	};
// 
// 	//Visibility2d m_visibility2d;
// };