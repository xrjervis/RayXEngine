#pragma once
#include "Game/GameCommon.hpp"
#include "Game/GameState.hpp"

class StopWatch;

class SetupState : public GameState {
public:
	SetupState(EscapeGame* game, EscapeWorld* world);
	virtual ~SetupState();

	virtual void OnEnter() override;
	virtual void OnExit() override;
	virtual void Update(float dt) override;
	virtual void Render() const override;

private:
	void SetDoorsAndWindowsOpenOrClose();
	void GenerateTreasures();

private:
	Uptr<StopWatch> m_endTurnStopWatch;

	int m_numMonitorsPlacedDown = 0;
	int m_numGuardsPlacedDown = 0;

	bool m_isThiefPlaced = false;

	bool m_isQuitting = false;

	std::vector<IntVector2> m_monitorGridCoords;
	std::vector<IntVector2> m_guardGridCoords;
};
