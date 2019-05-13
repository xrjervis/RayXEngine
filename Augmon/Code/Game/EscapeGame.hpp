#pragma once
#include "Game/GameCommon.hpp"

class Clock;
class GameState;
class EscapeWorld;
class SpriteSheet;

class EscapeGame {
public:
	EscapeGame();
	~EscapeGame();

	void Update();
	void Render() const;

	void AddGameState(const std::string& name);
	void ChangeToState(const std::string& name);

	IntVector2 GetMouseCursorCoordsInThiefWindow() const;

	void ClearThiefWindow();
	void ClearBoardWindow();
	void RebuildTileMesh();
	void RenderMapOnThiefWindow() const;
	void RenderMapOnBoardWindow() const;
	void RenderGridLinesOnBoardWindow() const;
	void WriteTextInBox(const std::string& text, float size, eTextAlignment horizontalAlign, eTextAlignment verticalAlign, const AABB2& box, const Rgba& color = Rgba::WHITE) const;
	void RenderInitialQuardsAtCorner() const;
	void RenderAvailavleTilesForSetupOnBoardWindow() const;
	void ShowAvailableTilesForThiefSetupOnThiefWindow() const;
	void RenderThiefCursorOnThiefWindow() const;
	void RenderThiefSpriteOnThiefWindow() const;
	void RenderGuardSpriteOnThiefWindow() const;
	void RenderAllTreasuresOnThiefWindow() const;
	void RenderKnownTreasuresOnBoardWindow() const;
	void RenderInfoAreaBackgroundOnBoardWindow() const;
	void RenderActionAreaBackgroundOnBoardWindow() const;
	void ShowAvailableTilesOnThiefWindow(const std::vector<IntVector2>& tiles) const;
	void ShowAvailableTilesOnBoardWindow(const std::vector<IntVector2>& tiles) const;
	void RenderThiefMovePathOnThiefWindow(const std::deque<IntVector2>& gridCoords) const;
	void RenderWhiteTilesForGuardsOnBoardWindow() const;
	void ShowCursorForGuardOnBoardWindow(int guardIdx) const;
	void RenderActionItemSlotOnBoardWindow() const;
	void RenderEquipmentSlotOnBoardWindow() const;
	void RenderActionPointOnBoardWindow(float isOn) const;
	void RenderEquipmentPointOnBoardWindow(float isOn) const;
	void RenderActionItemPatternOnBoardWindow(eActionType actionItem, const IntVector2& gridCoords, float width);
	void RenderAllMonitorsOnThiefWindow() const;
	void RenderKnownMonitorsOnBoardWindow() const;
	void ShowAvailableTilesForKnownMonitorsOnBoard() const;
	void RenderThiefSpriteOnBoardWindow() const;
	void RenderBatteryOnBoardWindow(int value) const;

	void RenderThiefWinOnThiefWindow() const;
	void RenderThiefWinOnBoardWindow() const;
	void RenderGuardWinOnThiefWindow() const;
	void RenderGuardWinOnBoardWindow() const;

	void ShowThiefIsRevealedOnBoardWindow(float normalizedTime, const Vector2& pos) const;
	void ShowClearOnBoardWindow(float normalizedTime, const Vector2& pos) const;
	void ShowPickUpTreasureOnThiefWindow(float normalizedTime, const Vector2& pos) const;
	void ShowDeactivateMonitorOnThiefWindow(float normalizedTime, const Vector2& pos) const;
	void ShowLockedOnThiefWindow(float normalizedTime, const Vector2& pos) const;

	void ShowTreasureIsStolenOnBoardWindow(float normalizedTime, const Vector2& pos) const;
	void ShowMonitorWasDeactivatedOnBoardWindow(float normalizedTime, const Vector2& pos) const;
	void ShowMonitorWasRecoveredOnBoardWindow(float normalizedTime, const Vector2& pos) const;
	void ShowNotEnoughTreasuresOnBoardWindow(float normalizedTime, const Vector2& pos) const;


public:
	Uptr<Clock> m_gameClock;
	Uptr<EscapeWorld> m_world;
	
	std::map<std::string, Uptr<GameState>> m_gameStates;
	GameState* m_currentState = nullptr;
	GameState* m_nextState = nullptr;

	Uptr<Mesh<VertexPCU>>				m_tileMeshOnBoard;
	Uptr<Mesh<VertexPCU>>				m_tileMeshOnThief;
	SpriteSheet*						m_tileSet;
};