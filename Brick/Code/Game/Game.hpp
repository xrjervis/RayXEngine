#pragma once
#include "Game/TileMap.hpp"
#include "Game/CollisionMap.hpp"
#include "Engine/Core/type.hpp"
#include <vector>
#include <unordered_map>

class TheApp;
class Clock;
class Camera;
class MIDIPlayer;


class Game {
public: 
	Game();
	~Game();

	void PostStartup();
	void Update();
	void Render() const;

	void LoadTileMap(const std::string& filePath);
	void LoadCollisionMap(const std::string& filePath);

	void UpdateInput();

	void DebugDrawGrids() const;

public:
	// game clock
	Uptr<Clock>										m_gameClock;
	Uptr<Camera>									m_mainCamera2D;
	//Uptr<Camera>									m_mainCamera3D;
	Uptr<MIDIPlayer>								m_midiPlayer;

	std::unordered_map<std::string, TileMap>		m_tileMaps;
	std::unordered_map<std::string, CollisionMap>	m_collisionMaps;
};