#pragma once
#include "Game/GameCommon.hpp"
#include "Game/GameState.hpp"

enum ePhase : int {
	PHASE_THIEF = 0,
	PHASE_GUARD_1,
	PHASE_GUARD_2,
	PHASE_GUARD_3,
};

class StopWatch;

class EncounterState : public GameState {
public:
	EncounterState(EscapeGame* game, EscapeWorld* world);
	virtual ~EncounterState();

	void Update(float ds) override;
	void Render() const override;
	void OnEnter() override;
	void OnExit() override;

	void OnThiefTurnStart();
	void OnGuardTurnStart(int guardIdx);

	void UpdateThiefTurn(float ds);
	void UpdateGuardTurn(float ds, int guardIdx);

	void WriteInstructionsOnThiefWindow() const;
	void WriteInstructionsOnBoardWindow() const;

	void WriteCurrentActionNameOnBoardWindow() const;
	void WriteCurrentEquipmentNameOnBoardWindow() const;

	void SelectAndCheckOneMonitor();
	void ScanAllMonitors();
	void CheckMotionDetector();

	void CheckIfThiefStepsOnTreasure();
	void CheckIfThiefStepsOnMonitor();
	void CheckIfThiefStepsOnEntrance();
	void CheckIfGuardStepsOnThief(const IntVector2& guardCoords);
	void CheckIfGuardStepsOnMonitor(const IntVector2& guardCoords);
	void UpdateIfGuardsCanSeeTreasuresAndThief();
	bool CheckIfMonitorWasDeactivated(const IntVector2& monitorCoords);

public:
	Uptr<StopWatch> m_endTurnStopWatch;
	ePhase m_currentPhase = PHASE_THIEF; 

	std::vector<IntVector2> m_grids;
	std::deque<IntVector2> m_thiefMovePath;

	bool m_isEndingTurn = false;
	bool m_isSelectingMonitor = false;
	bool m_isShowingRoomHasThief = false;
	bool m_needRemoveItems = false;
	Uptr<StopWatch> m_timer;
};