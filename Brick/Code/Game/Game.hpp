#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/type.hpp"
#include <vector>

class TheApp;
class Clock;
class Camera;
class Manager;
class MIDIPlayer;
class Entity;
class Brick;
class Ball;
class Paddle;


class Game {
public: 
	Game();
	~Game();

	void PostStartup();

	void Update();
	void Render() const;

public:
	// game clock
	Uptr<Clock>				m_gameClock;
	Uptr<Camera>			m_mainCamera2D;
	//Uptr<Camera>	m_mainCamera3D;

	Uptr<MIDIPlayer>		m_midiPlayer;

	std::vector<Brick*>	m_bricks;
	Ball*				m_ball = nullptr;
	Paddle*				m_paddle = nullptr;
};