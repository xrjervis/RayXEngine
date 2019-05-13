#include "Game/VictoryState.hpp"
#include "Game/EscapeGame.hpp"
#include "Game/EscapeWorld.hpp"

VictoryState::VictoryState(EscapeGame* game, EscapeWorld* world)
	: GameState(game, world) {

}

VictoryState::~VictoryState() {

}

void VictoryState::OnEnter() {

}

void VictoryState::OnExit() {

}

void VictoryState::Update(float dt) {

}

void VictoryState::Render() const {
	//-------------------------------Thief------------------------------
	m_game->ClearThiefWindow();
	m_game->RenderMapOnThiefWindow();
	m_game->RenderAllTreasuresOnThiefWindow();
	m_game->RenderAllMonitorsOnThiefWindow();
	m_game->RenderThiefSpriteOnThiefWindow();
	m_game->RenderGuardSpriteOnThiefWindow();
	m_game->RenderThiefCursorOnThiefWindow();
	if (m_world->m_doesThiefWin) {
		m_game->RenderThiefWinOnThiefWindow();
	}
	else {
		m_game->RenderGuardWinOnThiefWindow();
	}


	//-------------------------------Guard------------------------------
	m_game->ClearBoardWindow();
	m_game->RenderMapOnBoardWindow();
	m_game->RenderKnownTreasuresOnBoardWindow();

	if (!m_world->m_thief.m_isHidden) {
		m_game->RenderThiefSpriteOnBoardWindow();
	}

	if (m_world->m_doesThiefWin) {
		m_game->RenderThiefWinOnBoardWindow();
	}
	else {
		m_game->RenderGuardWinOnBoardWindow();
	}
}
