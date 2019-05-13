#pragma once
#include "Game/GameCommon.hpp"
#include "Game/GameState.hpp"

class VictoryState : public GameState {
public:
	VictoryState(EscapeGame* game, EscapeWorld* world);
	virtual ~VictoryState();

	virtual void OnEnter() override;
	virtual void OnExit() override;
	virtual void Update(float dt) override;
	virtual void Render() const override;

};