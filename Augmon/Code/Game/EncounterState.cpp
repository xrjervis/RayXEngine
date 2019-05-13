#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Game/EncounterState.hpp"
#include "Game/EscapeGame.hpp"
#include "Game/EscapeWorld.hpp"



EncounterState::EncounterState(EscapeGame* game, EscapeWorld* world)
	: GameState(game, world) {
	m_endTurnStopWatch = std::make_unique<StopWatch>(game->m_gameClock.get());
	m_endTurnStopWatch->SetTimer(world->m_gameConfig->GetValue("EndTurnButtonHoldSeconds", 3.f));

	m_timer = std::make_unique<StopWatch>(game->m_gameClock.get());
}

EncounterState::~EncounterState() {

}

void EncounterState::Update(float ds) {
	// Update Thief Cursor
	m_world->m_thief.m_cursorGridCoords = m_game->GetMouseCursorCoordsInThiefWindow();

	if (m_currentPhase == PHASE_THIEF) {
		UpdateThiefTurn(ds);
	}
	else {
		UpdateGuardTurn(ds, m_currentPhase - 1);
	}

	UpdateIfGuardsCanSeeTreasuresAndThief();

	// End turn button
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
		m_endTurnStopWatch->Reset();
		if (!m_world->IsPatternThere(EscapeWorld::s_emptyPattern, EscapeWorld::s_actionItemSlotBounds.mins.GetAsIntVector2()) ||
			!m_world->IsPatternThere(EscapeWorld::s_emptyPattern, EscapeWorld::s_equipmentSlotBounds.mins.GetAsIntVector2())) {
			m_needRemoveItems = true;
		}
		else {
			if (m_currentPhase == PHASE_THIEF) {
				if (m_world->m_thief.m_hasMovedThisTurn) {
					m_isEndingTurn = true;
				}
			}
			else {
				m_isEndingTurn = true;
			}
		}

	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_SPACE)) {
		if (m_currentPhase == PHASE_THIEF) {
			if (m_world->m_thief.m_hasMovedThisTurn) {
				if (m_endTurnStopWatch->HasElapsed() && m_isEndingTurn) {
					m_currentPhase = ePhase(m_currentPhase + 1);
					OnGuardTurnStart(m_currentPhase - 1);
					m_isEndingTurn = false;
				}
			}
		}
		else if(m_currentPhase >= PHASE_GUARD_1){
			if (m_endTurnStopWatch->HasElapsed() && m_isEndingTurn) {
				if (m_currentPhase == m_world->m_guards.size()) {
					m_currentPhase = PHASE_THIEF;
					OnThiefTurnStart();
					m_isEndingTurn = false;
				}
				else {
					m_currentPhase = ePhase(m_currentPhase + 1);
					OnGuardTurnStart(m_currentPhase - 1);
					m_isEndingTurn = false;
				}
			}
		}
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_SPACE)) {
		m_isEndingTurn = false;
		m_needRemoveItems = false;
	}
}

void EncounterState::Render() const {
	//-------------------------------Thief------------------------------
	m_game->ClearThiefWindow();
	m_game->RenderMapOnThiefWindow();
	m_game->RenderAllTreasuresOnThiefWindow();
	m_game->RenderAllMonitorsOnThiefWindow();
	m_game->RenderThiefSpriteOnThiefWindow();
	m_game->RenderGuardSpriteOnThiefWindow();
	if (m_currentPhase == PHASE_THIEF) {
		if (!m_world->m_thief.m_hasMovedThisTurn && !m_world->m_thief.m_isMoving) {
			m_game->ShowAvailableTilesOnThiefWindow(m_grids);
		}
	}
	m_game->RenderThiefCursorOnThiefWindow();
	m_game->RenderThiefMovePathOnThiefWindow(m_thiefMovePath);
	WriteInstructionsOnThiefWindow();
	

	//-------------------------------Guard------------------------------
	Guard* guard = &m_world->m_guards[m_currentPhase - 1];

	m_game->ClearBoardWindow();
	m_game->RenderMapOnBoardWindow();
	m_game->RenderInfoAreaBackgroundOnBoardWindow();
	m_game->RenderActionAreaBackgroundOnBoardWindow();
	m_game->RenderActionItemSlotOnBoardWindow();
	m_game->RenderEquipmentSlotOnBoardWindow();
	m_game->RenderKnownTreasuresOnBoardWindow();

	if (!m_world->m_thief.m_isHidden) {
		m_game->RenderThiefSpriteOnBoardWindow();
	}

	if (m_currentPhase != PHASE_THIEF) {
		if (guard->m_usedActionThisTurn) {
			WriteCurrentActionNameOnBoardWindow();
		}
		m_game->RenderActionPointOnBoardWindow(!guard->m_usedActionThisTurn);
		if (guard->m_usedEquipmentThisTurn) {
			WriteCurrentEquipmentNameOnBoardWindow();
			m_game->RenderBatteryOnBoardWindow(guard->m_flashLightRemainedBattery);
		}
		m_game->RenderEquipmentPointOnBoardWindow(!guard->m_usedEquipmentThisTurn);

		if (!guard->m_isMoving) {
			m_game->ShowCursorForGuardOnBoardWindow(m_currentPhase - 1);
		}
		else {
			m_game->ShowAvailableTilesOnBoardWindow(m_grids);
		}
	}

	if (m_isSelectingMonitor) {
		m_game->ShowAvailableTilesForKnownMonitorsOnBoard();
	}
	else {
		if (m_isShowingRoomHasThief) {
			if (!m_timer->HasElapsed()) {
				m_game->ShowAvailableTilesOnBoardWindow(m_grids);
			}
		}
		m_game->RenderKnownMonitorsOnBoardWindow();
	}

	m_game->RenderGridLinesOnBoardWindow();
	m_game->RenderWhiteTilesForGuardsOnBoardWindow();
	WriteInstructionsOnBoardWindow();
}

void EncounterState::OnEnter() {
	OnThiefTurnStart();
}

void EncounterState::OnExit() {

}

void EncounterState::OnThiefTurnStart() {
	m_grids.clear();
	m_world->FindMovableGridsWithinRange(m_world->m_thief.m_gridCoords, Thief::s_maxMoveDistance, m_grids);
	m_world->m_thief.m_isMoving = false;
	m_world->m_thief.m_hasMovedThisTurn = false;
	m_world->m_thief.m_isHidden = true;
	m_world->m_thief.m_fractionOfExposion = 0.f;

	// Start of a world turn
	m_world->m_totalTurns++;
}

void EncounterState::OnGuardTurnStart(int guardIdx) {
	m_grids.clear();
	Guard* guard = &m_world->m_guards[guardIdx];

	guard->m_needRemoveActionItems = false;
	guard->m_hasMovedThisTurn = false;
	guard->m_usedActionThisTurn = false;
	guard->m_isMoving = false;
	guard->m_usedEquipmentThisTurn = false;
	guard->m_isFlashlightOn = false;
	guard->m_maxMoveDistanceThisTurn = Guard::s_moveDistanceRange.GetRandomInRange();
	
	float randomValue = GetRandomFloatZeroToOne();
	if (randomValue <= Guard::s_possibilityForMotionDetector) {
		if (m_world->m_totalTurns > 1) {
			guard->m_actionGotThisTurn = ACTION_TYPE_MOTIONDETECTOR;
		}
		else {
			guard->m_actionGotThisTurn = ACTION_TYPE_SCANALLMONITORS;
		}
	}
	else if (randomValue <= Guard::s_possibilityForScanAllMonitors) {
		guard->m_actionGotThisTurn = ACTION_TYPE_SCANALLMONITORS;
	}
	else {
		guard->m_actionGotThisTurn = ACTION_TYPE_SINGLEMONITOR;
	}

	guard->m_actionsInHand.push_back(guard->m_actionGotThisTurn);
}

void EncounterState::UpdateThiefTurn(float ds) {
	if (!m_world->m_thief.m_hasMovedThisTurn && !m_world->m_thief.m_isMoving) {
		if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_LEFT)) {
			if (m_world->m_thief.m_cursorGridCoords == m_world->m_thief.m_gridCoords) {
				m_thiefMovePath.push_back(m_world->m_thief.m_cursorGridCoords);
			}
		}
		if (!m_thiefMovePath.empty() && m_thiefMovePath.front() == m_world->m_thief.m_gridCoords) {
			if (g_theInput->IsMousePressed(InputSystem::MOUSE_LEFT)) {
				const IntVector2& prevCoords = m_thiefMovePath[m_thiefMovePath.size() - 1];
				const IntVector2& curCoords = m_world->m_thief.m_cursorGridCoords;

				// Check if curCoords is in available grids
				if (curCoords != prevCoords && std::find(m_grids.begin(), m_grids.end(), curCoords) == m_grids.end()) {
					m_thiefMovePath.clear();
				}
				else if (std::find(m_thiefMovePath.begin(), m_thiefMovePath.end(), curCoords) == m_thiefMovePath.end()) {
					const IntVector2& distance = curCoords - prevCoords;
					if (Abs(distance.x) + Abs(distance.y) == 1) {
						m_thiefMovePath.push_back(curCoords);
					}
				}
			}
		}

		if (g_theInput->WasMouseJustReleased(InputSystem::MOUSE_LEFT)) {
			if (m_thiefMovePath.size() >= 2) {
				m_world->m_thief.m_isMoving = true;
			}
		}
	}
	else if (m_world->m_thief.m_isMoving) {
		Vector2 targetPosOnThiefWindow = m_world->GetGridCenterFromGridCoordsOnThiefWindow(m_thiefMovePath.front());
		Vector2 currentPosOnThiefWindow = m_world->m_thief.m_centerOnThiefWindow;

		Vector2 targetPosOnBoardWindow = m_world->GetGridCenterFromGridCoordsOnBoardWindow(m_thiefMovePath.front());
		Vector2 currentPosOnBoardWindow = m_world->m_thief.m_centerOnBoardWindow;

		currentPosOnThiefWindow = MoveTowards(currentPosOnThiefWindow, targetPosOnThiefWindow, ds * m_world->m_gridWidthInThiefWindow);
		m_world->m_thief.m_centerOnThiefWindow = currentPosOnThiefWindow;

		currentPosOnBoardWindow = MoveTowards(currentPosOnBoardWindow, targetPosOnBoardWindow, ds * GRID_WIDTH_IN_BOARD_WINDOW);
		m_world->m_thief.m_centerOnBoardWindow = currentPosOnBoardWindow;

		if (currentPosOnThiefWindow == targetPosOnThiefWindow && currentPosOnBoardWindow == targetPosOnBoardWindow) {
			m_world->m_thief.m_gridCoords = m_thiefMovePath.front();
			// Check treasure
			CheckIfThiefStepsOnTreasure();

			// Check monitor
			CheckIfThiefStepsOnMonitor();

			// Check entrance
			if (m_thiefMovePath.size() <= 1) {
				CheckIfThiefStepsOnEntrance();
			}

			m_thiefMovePath.pop_front();
			if (m_thiefMovePath.empty()) {
				m_world->m_thief.m_isMoving = false;
				m_world->m_thief.m_hasMovedThisTurn = true;
			}
		}
	}
}

//--------------------------------------------------------------------
void EncounterState::UpdateGuardTurn(float ds, int guardIdx) {
	Guard* guard = &m_world->m_guards[guardIdx];
	if (m_isSelectingMonitor) {
		SelectAndCheckOneMonitor();
	}
	else if (m_isShowingRoomHasThief) {
		if (m_timer->HasElapsed()) {
			m_isShowingRoomHasThief = false;
		}
	}
	else {

		if (!guard->m_hasMovedThisTurn) {
			if (!guard->m_isMoving) {
				if (!m_world->IsPatternThere(Guard::s_legoPattern, guard->m_gridCoords)) {
					guard->m_isMoving = true;
					m_world->FindMovableGridsWithinRange(guard->m_gridCoords, guard->m_maxMoveDistanceThisTurn, m_grids);
				}
			}
			else {
				for (auto gridCoords : m_grids) {
					if (m_world->IsPatternThere(Guard::s_legoPattern, gridCoords)) {
						guard->m_isMoving = false;
						guard->m_hasMovedThisTurn = true;
						guard->m_gridCoords = gridCoords;
						
						// Check capture thief
						CheckIfGuardStepsOnThief(gridCoords);

						// Check recover monitor
						CheckIfGuardStepsOnMonitor(gridCoords);

					}
				}
			}
		}
		if (!guard->m_usedActionThisTurn) {
			if (m_world->IsPatternThere(EscapeWorld::s_singleMonitorPattern, EscapeWorld::s_actionItemSlotBounds.mins.GetAsIntVector2())) {
				guard->m_usedActionThisTurn = true;
				guard->m_actionUsedThisTurn = ACTION_TYPE_SINGLEMONITOR;
				auto it = std::find(guard->m_actionsInHand.begin(), guard->m_actionsInHand.end(), guard->m_actionUsedThisTurn);
				if (it != guard->m_actionsInHand.end()) {
					guard->m_actionsInHand.erase(it);
				}
				m_isSelectingMonitor = true;
			}
			else if (m_world->IsPatternThere(EscapeWorld::s_scanAllMonitorsPattern, EscapeWorld::s_actionItemSlotBounds.mins.GetAsIntVector2())) {
				guard->m_usedActionThisTurn = true;
				guard->m_actionUsedThisTurn = ACTION_TYPE_SCANALLMONITORS;
				auto it = std::find(guard->m_actionsInHand.begin(), guard->m_actionsInHand.end(), guard->m_actionUsedThisTurn);
				if (it != guard->m_actionsInHand.end()) {
					guard->m_actionsInHand.erase(it);
				}
				ScanAllMonitors();
			}
			else if (m_world->IsPatternThere(EscapeWorld::s_motionDetectorPattern, EscapeWorld::s_actionItemSlotBounds.mins.GetAsIntVector2())) {
				guard->m_usedActionThisTurn = true;
				guard->m_actionUsedThisTurn = ACTION_TYPE_MOTIONDETECTOR;
				auto it = std::find(guard->m_actionsInHand.begin(), guard->m_actionsInHand.end(), guard->m_actionUsedThisTurn);
				if (it != guard->m_actionsInHand.end()) {
					guard->m_actionsInHand.erase(it);
				}
				CheckMotionDetector();
			}
		}
		if (!guard->m_usedEquipmentThisTurn) {
			if (guard->m_flashLightRemainedBattery > 0) {
				if (m_world->IsPatternThere(EscapeWorld::s_flashLightPattern, EscapeWorld::s_equipmentSlotBounds.mins.GetAsIntVector2())) {
					guard->m_usedEquipmentThisTurn = true;
					guard->m_isFlashlightOn = true;
					guard->m_flashLightRemainedBattery--;
				}
			}
		}
	}
}

void EncounterState::WriteInstructionsOnThiefWindow() const {
	if (m_isEndingTurn) {
		std::string info = Stringf("Ready to Start? %d", (int)(m_world->m_gameConfig->GetValue("EndTurnButtonHoldSeconds", 3.f) - m_endTurnStopWatch->GetElapsedTime() + 1.f));
		m_game->WriteTextInBox(info, 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
	}
	else {
		if (m_currentPhase != PHASE_THIEF) {
			m_game->WriteTextInBox("Waiting for Guards...", 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
		}
		else {
			if (!m_world->m_thief.m_hasMovedThisTurn) {
				m_game->WriteTextInBox("Your Turn", 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
			}
			else {
				m_game->WriteTextInBox("Press and hold SPACE to end turn", 32.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP, AABB2(Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(m_world->m_gridWidthInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.x, m_world->m_gridHeightInThiefWindow * EscapeWorld::s_infoAreaBounds.maxs.y)));
			}
		}
	}
}

void EncounterState::WriteInstructionsOnBoardWindow() const {
	if (m_needRemoveItems) {
		m_game->WriteTextInBox("Need to remove action item or equipment item.", 48.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.y)));
	}
	else if (m_isEndingTurn) {
		std::string info = Stringf("Ready to Start? %d", (int)(m_world->m_gameConfig->GetValue("EndTurnButtonHoldSeconds", 3.f) - m_endTurnStopWatch->GetElapsedTime() + 1.f));
		m_game->WriteTextInBox(info, 48.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.maxs.y)));
	}
	else {
		if (m_currentPhase != PHASE_THIEF) {

			Guard* guard = &m_world->m_guards[m_currentPhase - 1];
			std::string info = Stringf("You drew an [%s] item.", m_world->GetStringFromActionType(guard->m_actionGotThisTurn).c_str());
			Vector2 boxMin = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_infoAreaBounds.mins.y + 2));
			Vector2 boxMax = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(GRID_COUNT_X / 2, EscapeWorld::s_infoAreaBounds.maxs.y));
			m_game->WriteTextInBox(info, 32.f, TEXT_ALIGN_RIGHT, TEXT_ALIGN_BOTTOM, AABB2(boxMin, boxMax), Rgba::WHITE);
			m_game->RenderActionItemPatternOnBoardWindow(guard->m_actionGotThisTurn, IntVector2(GRID_COUNT_X / 2 + 1, EscapeWorld::s_infoAreaBounds.mins.y + 2), 2.f * GRID_WIDTH_IN_BOARD_WINDOW);

			// Write move distance this turn
			info = Stringf("Move distance this turn: %d", guard->m_maxMoveDistanceThisTurn);
			boxMin = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(GRID_COUNT_X / 2 + 3, EscapeWorld::s_infoAreaBounds.mins.y + 2));
			boxMax = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(GRID_COUNT_X, EscapeWorld::s_infoAreaBounds.maxs.y));
			m_game->WriteTextInBox(info, 32.f, TEXT_ALIGN_LEFT, TEXT_ALIGN_BOTTOM, AABB2(boxMin, boxMax), Rgba::WHITE);


			// Render guards actions in hand
			boxMin = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_infoAreaBounds.mins.y));
			boxMax = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(GRID_COUNT_X / 2, EscapeWorld::s_infoAreaBounds.mins.y + 1));
			m_game->WriteTextInBox("Available actions:", 24.f, TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, AABB2(boxMin, boxMax), Rgba::WHITE);

			for (int i = 0; i < guard->m_actionsInHand.size(); ++i) {
				eActionType actionType = guard->m_actionsInHand[i];
				m_game->RenderActionItemPatternOnBoardWindow(actionType, IntVector2((int)EscapeWorld::s_infoAreaBounds.mins.x + 5 + i, (int)EscapeWorld::s_infoAreaBounds.mins.y), GRID_WIDTH_IN_BOARD_WINDOW);
			}
		}
		else {
			Vector2 boxMin = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_infoAreaBounds.mins.y));
			Vector2 boxMax = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(GRID_COUNT_X, EscapeWorld::s_infoAreaBounds.maxs.y));
			m_game->WriteTextInBox("Waiting for Thief...", 48.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(boxMin, boxMax), Rgba::WHITE);
		}
	}
}

void EncounterState::WriteCurrentActionNameOnBoardWindow() const {
	Guard* guard = &m_world->m_guards[m_currentPhase - 1];
	Vector2 boxMins = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_actionItemSlotBounds.mins.y - 1));
	Vector2 boxMaxs = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(4, EscapeWorld::s_actionItemSlotBounds.mins.y));

	m_game->WriteTextInBox(m_world->GetStringFromActionType(guard->m_actionUsedThisTurn), 16.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(boxMins, boxMaxs), Rgba::CYAN);
}

void EncounterState::WriteCurrentEquipmentNameOnBoardWindow() const {
	Vector2 boxMins = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_equipmentSlotBounds.mins.y - 1));
	Vector2 boxMaxs = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(4, EscapeWorld::s_equipmentSlotBounds.mins.y));

	m_game->WriteTextInBox("Flashlight", 16.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(boxMins, boxMaxs), Rgba::CYAN);
}

void EncounterState::SelectAndCheckOneMonitor() {
	for (auto m : m_world->m_monitorsGuardKnows) {
		if (m.second) {
			IntVector2 gridCoords = m.first;

			float depthValueAtGridCoords = m_world->GetDepthValueAtGridCoords(gridCoords);
			if (0.54f < depthValueAtGridCoords && depthValueAtGridCoords < 0.55f) {
				Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(gridCoords) + Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW * 0.5f);

				bool wasDeactivated = CheckIfMonitorWasDeactivated(gridCoords);
				if (wasDeactivated) {
					g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowMonitorWasDeactivatedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
				}
				else {
					bool success = m_world->CheckIfMonitorCanSeeThief(gridCoords);
					if (success) {
						g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowThiefIsRevealedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
						m_world->m_thief.m_isHidden = false;
					}
					else {
						g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowClearOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
					}
				}

				m_isSelectingMonitor = false;
				break;
			}
		}
	}
}

void EncounterState::ScanAllMonitors() {
	for (auto m : m_world->m_monitorsGuardKnows) {
		if (m.second) {
			IntVector2 gridCoords = m.first;
			Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(gridCoords) + Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW * 0.5f);

			bool wasDeactivated = CheckIfMonitorWasDeactivated(gridCoords);
			if (wasDeactivated) {
				g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowMonitorWasDeactivatedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
			}
			else {
				bool success = m_world->CheckIfMonitorCanSeeThief(gridCoords);
				if (success) {
					g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowThiefIsRevealedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
					m_world->m_thief.m_isHidden = false;
				}
				else {
					g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowClearOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
				}
			}
		}
	}
}

void EncounterState::CheckMotionDetector() {
	bool result = m_world->CheckIfThiefIsInRoom(m_grids);
	if (result) {
		m_isShowingRoomHasThief = true;
		m_timer->SetTimer(5.f);
		m_timer->Reset();
	}
	else {
		Guard* guard = &m_world->m_guards[m_currentPhase - 1];
		Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(guard->m_gridCoords) + 0.5f * Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW);
		g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowClearOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
	}
}

void EncounterState::CheckIfThiefStepsOnTreasure() {
	IntVector2 thiefCoords = m_world->m_thief.m_gridCoords;
	for (auto& t : m_world->m_treasures) {
		if (!t.second) {
			if (t.first == thiefCoords) {
				t.second = true;
				m_world->m_thief.m_numTreasuresStolen++;
				Vector2 pos = m_world->m_thief.m_centerOnThiefWindow + 0.5f * Vector2(0.f, m_world->m_gridHeightInThiefWindow);
				g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowPickUpTreasureOnThiefWindow, m_game, std::placeholders::_1, std::placeholders::_2));
			}
		}
	}
}

void EncounterState::CheckIfThiefStepsOnMonitor() {
	IntVector2 thiefCoords = m_world->m_thief.m_gridCoords;
	for (auto& m : m_world->m_monitors) {
		if (m.second) {
			if (m.first == thiefCoords) {
				m.second = false;
				Vector2 pos = m_world->m_thief.m_centerOnThiefWindow + 0.5f * Vector2(0.f, m_world->m_gridHeightInThiefWindow);
				g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowDeactivateMonitorOnThiefWindow, m_game, std::placeholders::_1, std::placeholders::_2));
			}
		}
	}
}

void EncounterState::CheckIfThiefStepsOnEntrance() {
	IntVector2 thiefCoords = m_world->m_thief.m_gridCoords;
	for (auto d : m_world->m_doorsAndWinodwsStates) {
		if (d.first == thiefCoords) {
			if (!d.second) {
				Vector2 pos = m_world->m_thief.m_centerOnThiefWindow + 0.5f * Vector2(0.f, m_world->m_gridHeightInThiefWindow);
				g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowLockedOnThiefWindow, m_game, std::placeholders::_1, std::placeholders::_2));
			}
			else if (m_world->m_thief.m_numTreasuresStolen < m_world->m_leastTreasuresStolenToWin) {
				Vector2 pos = m_world->m_thief.m_centerOnThiefWindow + 0.5f * Vector2(0.f, m_world->m_gridHeightInThiefWindow);
				g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowNotEnoughTreasuresOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
			}
			else {
				m_world->m_doesThiefWin = true;
				m_game->ChangeToState("Victory");
			}
		}
	}
}

void EncounterState::CheckIfGuardStepsOnThief(const IntVector2& guardCoords) {
	if (guardCoords == m_world->m_thief.m_gridCoords) {
		m_world->m_doesThiefWin = false;
		m_game->ChangeToState("Victory");
	}
}

void EncounterState::CheckIfGuardStepsOnMonitor(const IntVector2& guardCoords) {
	if (m_world->m_monitors.find(guardCoords) != m_world->m_monitors.end()) {
		if (m_world->m_monitors.at(guardCoords) == false) {
			m_world->m_monitors[guardCoords] = true;
			m_world->m_monitorsGuardKnows[guardCoords] = true;
			Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(guardCoords) + 0.5f * Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW);
			g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowMonitorWasRecoveredOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
		}
	}
}

void EncounterState::UpdateIfGuardsCanSeeTreasuresAndThief() {
	for (auto& guard : m_world->m_guards) {
		Vector2 guardCenter = m_world->GetGridCenterFromGridCoordsOnBoardWindow(guard.m_gridCoords);
		Vector2 bl_guard = guardCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 br_guard = guardCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 tl_guard = guardCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 tr_guard = guardCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

		// Check thief
		Vector2 thiefCenter = m_world->m_thief.m_centerOnBoardWindow;
		Vector2 bl_thief = thiefCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 br_thief = thiefCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 tl_thief = thiefCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 tr_thief = thiefCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

		{
			Vector2 bl_dir = (bl_thief - bl_guard).GetNormalized();
			Vector2 br_dir = (br_thief - br_guard).GetNormalized();
			Vector2 tl_dir = (tl_thief - tl_guard).GetNormalized();
			Vector2 tr_dir = (tr_thief - tr_guard).GetNormalized();

			float bl_distance = GetDistance(bl_thief, bl_guard);
			float br_distance = GetDistance(br_thief, br_guard);
			float tl_distance = GetDistance(tl_thief, tl_guard);
			float tr_distance = GetDistance(tr_thief, tr_guard);                  

			RaycastResult2D_t bl_result = m_world->Raycast(bl_guard, bl_dir, bl_distance);
			RaycastResult2D_t br_result = m_world->Raycast(br_guard, br_dir, br_distance);
			RaycastResult2D_t tl_result = m_world->Raycast(tl_guard, tl_dir, tl_distance);
			RaycastResult2D_t tr_result = m_world->Raycast(tr_guard, tr_dir, tr_distance);

// 			DebugDrawLine(0.f, bl_guard, bl_thief, Rgba::YELLOW, Rgba::YELLOW);
// 			DebugDrawLine(0.f, br_guard, br_thief, Rgba::YELLOW, Rgba::YELLOW);
// 			DebugDrawLine(0.f, tl_guard, tl_thief, Rgba::YELLOW, Rgba::YELLOW);
// 			DebugDrawLine(0.f, tr_guard, tr_thief, Rgba::YELLOW, Rgba::YELLOW);

			bool result = false;
			m_world->m_thief.m_fractionOfExposion = 0.f;
			if (!bl_result.m_didImpact) {
				m_world->m_thief.m_fractionOfExposion += 0.25f;
				result = true;
			}
			if (!br_result.m_didImpact) {
				m_world->m_thief.m_fractionOfExposion += 0.25f;
				result = true;
			}
			if (!tl_result.m_didImpact) {
				m_world->m_thief.m_fractionOfExposion += 0.25f;
				result = true;
			}
			if (!tr_result.m_didImpact) {
				m_world->m_thief.m_fractionOfExposion += 0.25f;
				result = true;
			}

			if (!guard.m_isFlashlightOn) {
				if (bl_distance > (float)Guard::s_alertDistanceDefault * GRID_WIDTH_IN_BOARD_WINDOW) {
					result = false;
				}
			}
			else {
				if (bl_distance > (float)Guard::s_alertDistanceWithFlashlight * GRID_WIDTH_IN_BOARD_WINDOW) {
					result = false;
				}
			}

			if (result) {
				if (m_world->m_thief.m_isHidden) {
					m_world->m_thief.m_isHidden = false;
// 					Vector2 pos = thiefCenter + 0.5f * Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW);
// 					g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowThiefIsRevealedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
				}
			}
			else {
				m_world->m_thief.m_isHidden = true;
			}
		}

		// Check treasures
		for (auto& t : m_world->m_treasures) {
			IntVector2 treasureCoords = t.first;
			Vector2 treasureCenter = m_world->GetGridCenterFromGridCoordsOnBoardWindow(treasureCoords);
			Vector2 bl_treasure = treasureCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
			Vector2 br_treasure = treasureCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
			Vector2 tl_treasure = treasureCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
			Vector2 tr_treasure = treasureCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

			{
				Vector2 bl_dir = (bl_treasure - bl_guard).GetNormalized();
				Vector2 br_dir = (br_treasure - br_guard).GetNormalized();
				Vector2 tl_dir = (tl_treasure - tl_guard).GetNormalized();
				Vector2 tr_dir = (tr_treasure - tr_guard).GetNormalized();

				float bl_distance = GetDistance(bl_treasure, bl_guard);
				float br_distance = GetDistance(br_treasure, br_guard);
				float tl_distance = GetDistance(tl_treasure, tl_guard);
				float tr_distance = GetDistance(tr_treasure, tr_guard);

				RaycastResult2D_t bl_result = m_world->Raycast(bl_guard, bl_dir, bl_distance);
				RaycastResult2D_t br_result = m_world->Raycast(br_guard, br_dir, br_distance);
				RaycastResult2D_t tl_result = m_world->Raycast(tl_guard, tl_dir, tl_distance);
				RaycastResult2D_t tr_result = m_world->Raycast(tr_guard, tr_dir, tr_distance);


				bool result = false;
				if (!bl_result.m_didImpact || !br_result.m_didImpact || !tl_result.m_didImpact || !tr_result.m_didImpact) {
					result = true;
				}

				if (!guard.m_isFlashlightOn) {
					if (bl_distance > (float)m_world->m_monitorAlertDistanceDefault * GRID_WIDTH_IN_BOARD_WINDOW) {
						result = false;
					}
				}

				if (result) {
					bool thiefTreasureOpen = t.second;
					bool guardTreasureOpen = m_world->m_treasuresGuardKnows.at(t.first);
					if (thiefTreasureOpen && !guardTreasureOpen) {
						m_world->m_treasuresGuardKnows.at(t.first) = true;
						Vector2 pos = treasureCenter + 0.5f * Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW);
						g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowTreasureIsStolenOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
					}
				}
			}
		}
	}
}

bool EncounterState::CheckIfMonitorWasDeactivated(const IntVector2& monitorCoords) {
	bool thiefMonitorActive = m_world->m_monitors[monitorCoords];
	bool guardMonitorActive = m_world->m_monitorsGuardKnows[monitorCoords];
	if (!thiefMonitorActive && guardMonitorActive) {
		m_world->m_monitorsGuardKnows[monitorCoords] = false;
		Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(monitorCoords) + 0.5f * Vector2(0.f, GRID_HEIGHT_IN_BOARD_WINDOW);
		g_tasks.emplace_back(5.f, m_game->m_gameClock.get(), pos, std::bind(&EscapeGame::ShowMonitorWasDeactivatedOnBoardWindow, m_game, std::placeholders::_1, std::placeholders::_2));
		return true;
	}
	return false;
}
