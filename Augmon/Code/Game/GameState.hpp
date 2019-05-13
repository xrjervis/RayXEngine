#pragma once
#include <string>

enum eGameState {
	GAME_ATTRACT,
	CAMERA_SETUP,
	GUARD_SETUP,
	THIEF_SETUP,
	THIEF_MOVE,
	GUARD_WAIT,
	GUARD_MOVE,
	GUARD_ACT,
	THIEF_WIN,
	GUARD_WIN,
	NUM_GAMESTATES
};

class EscapeWorld;
class EscapeGame;

class GameState {
public:
	GameState(EscapeGame* game, EscapeWorld* world) : m_game(game), m_world(world) {};
	virtual ~GameState() = default;

	virtual void Update(float dt) = 0;
	virtual void Render() const = 0;
	virtual void OnEnter() = 0;
	virtual void OnExit() = 0;

public:
	EscapeGame* m_game = nullptr;
	EscapeWorld* m_world = nullptr;
	bool m_isReady = false;
};