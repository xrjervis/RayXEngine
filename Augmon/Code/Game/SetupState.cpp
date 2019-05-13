#include "Game/SetupState.hpp"
#include "Game/Thief.hpp"
#include "Game/Board.hpp"
#include "Game/EscapeWorld.hpp"
#include "Game/EscapeGame.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/InputSystem/InputSystem.hpp"

SetupState::SetupState(EscapeGame* game, EscapeWorld* world)
	: GameState(game, world) {
	m_endTurnStopWatch = std::make_unique<StopWatch>(game->m_gameClock.get());
	m_endTurnStopWatch->SetTimer(world->m_gameConfig->GetValue("EndTurnButtonHoldSeconds", 3.f));

	Thief::s_maxMoveDistance = m_world->m_gameConfig->GetValue("ThiefMaxMoveDistance", 1);
	Guard::s_moveDistanceRange = m_world->m_gameConfig->GetValue("GuardMoveDistanceRange", IntRange());
	Guard::s_flashlightFullBattery = m_world->m_gameConfig->GetValue("GuardFlashlightFullBattery", 0);
	Guard::s_possibilityForSingleMonitor = m_world->m_gameConfig->GetValue("PossibilityToDrawSingleMonitorAction", 0.f);
	Guard::s_possibilityForScanAllMonitors = m_world->m_gameConfig->GetValue("PossibilityToDrawScanAllMonitorsAction", 0.f);
	Guard::s_possibilityForMotionDetector = m_world->m_gameConfig->GetValue("PossibilityToDrawMotionDetectorAction", 0.f);
	Guard::s_alertDistanceDefault = m_world->m_gameConfig->GetValue("GuardAlertDistanceDefault", 0.f);
	Guard::s_alertDistanceWithFlashlight = m_world->m_gameConfig->GetValue("GuardAlertDistanceWithFlashlight", 0.f);
}

SetupState::~SetupState() {

}

void SetupState::OnEnter() {
	SetDoorsAndWindowsOpenOrClose();
	GenerateTreasures();

	m_world->m_monitors.clear();
	m_world->m_monitorsGuardKnows.clear();
	m_world->m_guards.clear();
}

void SetupState::OnExit() {
	// Guard setup
	for (int i = 0; i < m_numGuardsPlacedDown; ++i) {
		m_world->m_guards.emplace_back();
		m_world->m_guards[i].m_gridCoords = m_guardGridCoords[i];
		m_world->m_guards[i].m_actionsInHand.clear();
		m_world->m_guards[i].m_flashLightRemainedBattery = Guard::s_flashlightFullBattery;
	}

	// Add monitors
	for (auto m : m_monitorGridCoords) {
		m_world->m_monitors.insert({ m, true });
		m_world->m_monitorsGuardKnows.insert({ m, true });
	}
}

void SetupState::Update(float dt) {
	if (m_world->m_board->m_isReady == false) {
		return;
	}

	//--------------------------------------------------------------------
	// Looking for monitors and guard pieces
	m_monitorGridCoords.clear();
	m_world->FindPatternInArea(EscapeWorld::s_monitorPattern, EscapeWorld::s_playableAreaBounds, m_monitorGridCoords);
	m_numMonitorsPlacedDown = m_monitorGridCoords.size();

	m_guardGridCoords.clear();
	m_world->FindPatternInArea(Guard::s_legoPattern, EscapeWorld::s_playableAreaBounds, m_guardGridCoords);
	m_numGuardsPlacedDown = m_guardGridCoords.size();


	// Update Thief Cursor
	m_world->m_thief.m_cursorGridCoords = m_game->GetMouseCursorCoordsInThiefWindow();

	// Thief setup
	if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_LEFT)) {
		if (!m_isThiefPlaced) {
			int clickGridIndex = m_world->m_thief.m_cursorGridCoords.y * GRID_COUNT_X + m_world->m_thief.m_cursorGridCoords.x;
			bool isEntrance = m_world->m_tiles[clickGridIndex].m_isEntrance;
			if (isEntrance) {
				m_world->m_thief.m_isHidden = true;
				m_world->m_thief.m_gridCoords = m_world->m_thief.m_cursorGridCoords;
				m_world->m_thief.m_centerOnThiefWindow = m_world->GetGridCenterFromGridCoordsOnThiefWindow(m_world->m_thief.m_gridCoords);
				m_world->m_thief.m_centerOnBoardWindow = m_world->GetGridCenterFromGridCoordsOnBoardWindow(m_world->m_thief.m_gridCoords);
				m_isThiefPlaced = true;

				// Guarantee thief's entry point and farthest point are open
				m_world->m_doorsAndWinodwsStates.at(m_world->m_thief.m_gridCoords) = true;
				IntVector2 farthestCoords;
				float maxDistanceSquared = 0.f;
				for (auto& it : m_world->m_doorsAndWinodwsStates) {
					float distSquared = GetDistanceSquared(Vector2(it.first), Vector2(m_world->m_thief.m_gridCoords));
					if (distSquared > maxDistanceSquared) {
						maxDistanceSquared = distSquared;
						farthestCoords = it.first;
					}
				}
				m_world->m_doorsAndWinodwsStates.at(farthestCoords) = true;
			}
		}
	}
	else if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_RIGHT)) {
		m_isThiefPlaced = false;
	}

	//--------------------------------------------------------------------
	// Check if setup is completed
	if (0 < m_numGuardsPlacedDown && m_numGuardsPlacedDown <= 3 &&
		m_numMonitorsPlacedDown == m_world->m_totalMonitors && m_isThiefPlaced) {
		m_isReady = true;
	}
	else {
		m_isReady = false;
	}

	//---------------------------------------------------------------------
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
		m_endTurnStopWatch->Reset();
	}

	m_isQuitting = false;
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_SPACE)) {
		if (m_isReady) {
			m_isQuitting = true;
			if (m_endTurnStopWatch->HasElapsed()) {
				m_game->ChangeToState("Encounter");
			}
		}
	}
}

void SetupState::Render() const {
	if (m_world->m_board->m_isReady == false) {
		//-------------------------------Thief------------------------------
		m_game->ClearThiefWindow();
		m_game->WriteTextInBox("Wait for initial alignment...", 48.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(Vector2::ZERO, Vector2(m_world->m_widthInThiefWindow, m_world->m_heightInThiefWindow)));


		//-------------------------------Guard------------------------------
		m_game->ClearBoardWindow();
		m_game->RenderInitialQuardsAtCorner();
	}
	else {
		std::string info;


		//-------------------------------Thief------------------------------
		m_game->ClearThiefWindow();
		m_game->RenderMapOnThiefWindow();
		m_game->RenderAllTreasuresOnThiefWindow();
		if (m_isThiefPlaced) {
			m_game->RenderThiefSpriteOnThiefWindow();
		}
		else {
			m_game->ShowAvailableTilesForThiefSetupOnThiefWindow();
		}
		m_game->RenderThiefCursorOnThiefWindow();

		if (m_isQuitting) {
			info = Stringf("Ready to Start? %d", (int)(m_world->m_gameConfig->GetValue("EndTurnButtonHoldSeconds", 3.f) - m_endTurnStopWatch->GetElapsedTime() + 1.f));
			m_game->WriteTextInBox(info, 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
		}
		else {
			if (m_isThiefPlaced) {
				info = "Press and hold [Space] to end turn\nPress [Right Mouse Button] to reset";
				m_game->WriteTextInBox(info, 30.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
			}
			else {
				info = "Press [Left Mouse Button] to place Thief";
				m_game->WriteTextInBox(info, 30.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
			}
		}

		//RenderDebugStuffInThiefWindow();

		//-------------------------------Guard------------------------------
		m_game->ClearBoardWindow();
		m_game->RenderMapOnBoardWindow();
		m_game->RenderInfoAreaBackgroundOnBoardWindow();
		m_game->RenderActionAreaBackgroundOnBoardWindow();
		m_game->RenderKnownTreasuresOnBoardWindow();
		m_game->RenderAvailavleTilesForSetupOnBoardWindow();
		m_game->RenderGridLinesOnBoardWindow();

		if(!m_isQuitting) {
			info = Stringf("Monitors: ");
			if (m_world->m_totalMonitors == m_numMonitorsPlacedDown) {
				info += "Ready";
			}
			else if (m_world->m_totalMonitors > m_numMonitorsPlacedDown) {
				info += Stringf("%d more", m_world->m_totalMonitors - m_numMonitorsPlacedDown);
			}
			else {
				info += Stringf("%d less", m_numMonitorsPlacedDown - m_world->m_totalMonitors);
			}
			info += Stringf("  Guards: %d", m_numGuardsPlacedDown);
		}

		m_game->WriteTextInBox(info, 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.y)));
	}
}

void SetupState::SetDoorsAndWindowsOpenOrClose() {
	m_world->m_doorsAndWinodwsStates.clear();

	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
		bool isEntrance = m_world->m_tiles[i].m_isEntrance;
		if (isEntrance) {
			int gridX = i % GRID_COUNT_X;
			int gridY = i / GRID_COUNT_X;
			bool isOpen = CheckRandomChance(0.2f);
			m_world->m_doorsAndWinodwsStates.insert({ IntVector2(gridX, gridY), isOpen });
		}
	}
}

void SetupState::GenerateTreasures() {
	m_world->m_treasures.clear();
	m_world->m_treasuresGuardKnows.clear();

	std::vector<IntVector2> floors;
	for (int gridY = 0; gridY < GRID_COUNT_Y; ++gridY) {
		for (int gridX = 0; gridX < GRID_COUNT_X; ++gridX) {
			int gridIndex = gridY * GRID_COUNT_X + gridX;
			int indexInTileSet = m_world->m_tiles[gridIndex].m_indexInTileset;
			if (indexInTileSet == 30) {
				floors.emplace_back(gridX, gridY);
			}
		}
	}
	while (m_world->m_treasures.size() < m_world->m_totalTreasures) {
		int index = GetRandomIntInRange(0, floors.size() - 1);
		int gridX = floors[index].x;
		int gridY = floors[index].y;

		bool flag = false;
		for (auto& t : m_world->m_treasures) {
			//--------------------------------------------------------------------
			// if there is any treasure around the new gridCoords, ignore this gridCoords
			if (Abs(t.first.x - gridX) <= 2 && Abs(t.first.y - gridY) <= 2) {
				flag = true;
				break;
			}
		}
		if (flag) {
			continue;
		}
		else {
			m_world->m_treasures.insert({ IntVector2(gridX, gridY), false });
			m_world->m_treasuresGuardKnows.insert({ IntVector2(gridX, gridY), false });
		}
	}
}

