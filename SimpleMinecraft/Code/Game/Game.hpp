#pragma once
#include "Engine/Core/type.hpp"

class Clock;
class Manager;
class World;

class Game {
public: 
	Game();
	~Game();

	void Update();
	void Render() const;

public:
	// game clock
	Uptr<Clock>		m_gameClock;

	Uptr<World>		m_world;
};
