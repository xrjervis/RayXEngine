#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/RHIDevice.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Engine/Renderer/d3d11/FontRenderer.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"


Game::Game(){
	//--------------------------------------------------------------------------
	// Set game coords matrix
	Matrix44::GameToEngine = Matrix44(Vector3(0.f, 0.f, 1.f), Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	Matrix44::EngineToGame = Matrix44::GameToEngine;
	Matrix44::EngineToGame.Inverse();

	//--------------------------------------------------------------------------
	// Create my game clock
	m_gameClock = std::make_unique<Clock>(g_theMasterClock.get());
	g_theMasterClock->AddChild(m_gameClock.get());

	//--------------------------------------------------------------------------
	// Create my world
	m_world = std::make_unique<World>();
}

Game::~Game() {

}

void Game::Update() {
	m_world->Update(m_gameClock->GetDeltaSeconds());
}

void Game::Render() const {
	m_world->Render();
}