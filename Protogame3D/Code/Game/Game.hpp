#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/type.hpp"

class TheApp;
class Clock;
class Camera;
class Manager;
class MIDIPlayer;
class Entity;

class Game {
public: 
	Game();
	~Game();

	void PostStartup();

	void Update();
	void Render() const;

	Entity* SpawnEntity(float age, const Vector3& pos, const Rgba& color);

public:
	// game clock
	Uptr<Clock>		m_gameClock;
	Uptr<Camera>	m_mainCamera2D;
	Uptr<Camera>	m_mainCamera3D;

	Uptr<MIDIPlayer> m_midiPlayer;

	std::vector<Uptr<Entity>> m_entities;
};