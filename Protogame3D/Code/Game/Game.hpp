#pragma once
#include "Engine/Core/type.hpp"

class TheApp;
class Clock;
class Camera;
class Manager;
class MIDIPlayer;

class Game {
public: 
	Game();
	~Game();

	void Update();
	void Render() const;


public:
	// game clock
	Uptr<Clock>		m_gameClock;
	Uptr<Camera>	m_mainCamera2D;
	Uptr<Camera>	m_mainCamera3D;

	Uptr<MIDIPlayer> m_midiPlayer;
};