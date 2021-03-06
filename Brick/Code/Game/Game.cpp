#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Audio/MIDIPlayer.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/SmoothNoise.hpp"
#include "Engine/Core/NamedFunctions.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/Game.hpp"
#include "Game/TheApp.hpp"
#include "Game/Entity2d.hpp"

Game::Game() {
//	---
//	// Set game coords matrix
// 	Matrix44::GameToEngine = Matrix44(Vector3(0.f, 0.f, 1.f), Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
// 	Matrix44::EngineToGame = Matrix44::GameToEngine;
// 	Matrix44::EngineToGame.Inverse();

	PostStartup();
}

Game::~Game() {
}

//--------------------------------------------------------------------
void Game::PostStartup() {
	// Create my game clock
	m_gameClock = std::make_unique<Clock>(g_theMasterClock.get());
	g_theMasterClock->AddChild(m_gameClock.get());

	// Create the main 2d camera
	m_mainCamera2D = std::make_unique<Camera>();
	m_mainCamera2D->SetViewport(0U, 0, 0, g_mainOutput->GetWidth(), g_mainOutput->GetHeight(), 0.f, 1.f);
	m_mainCamera2D->SetProjectionMode(ORTHOGRAPHIC);
	m_mainCamera2D->SetOrtho(9.f, 16.f / 9.f, 0.f, 10.f);
	m_mainCamera2D->SetRenderTarget(g_mainOutput->GetRTV());
	m_mainCamera2D->SetDepthTarget(g_mainOutput->GetDSV());

	// Setup debug draw system to use my 2d/3d cameras
	g_theDebugDrawSystem->SetCamera2D(m_mainCamera2D.get());

	// Create midi player
	m_midiPlayer = std::make_unique<MIDIPlayer>();

}

//--------------------------------------------------------------------
void Game::Update() {
	UpdateInput();

}

//--------------------------------------------------------------------
void Game::Render() const {
	// Setup render pipeline
	g_theRHI->GetFontRenderer()->BindOutput(g_mainOutput.get());
	g_theRHI->GetDevice()->ClearColor(g_mainOutput->GetRTV(), Rgba::MIDNIGHTBLUE);
	g_theRHI->GetDevice()->ClearDepthStencil(g_mainOutput->GetDSV(), 1.f, 0U);

	// Hook render target
	g_theRHI->GetImmediateRenderer()->BindCamera(m_mainCamera2D.get());
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
/*	g_theRHI->GetImmediateRenderer()->DrawQuad2D(m_mainCamera2D->GetOrtho().GetCenter(), Vector2(0.5f, 0.5f), Vector2(50.f, 50.f), Rgba::YELLOW);*/

	// Setup font renderer
	g_theRHI->GetFontRenderer()->SetFont(L"Fira Code");
	g_theRHI->GetFontRenderer()->SetSize(32.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);

	DebugDrawGrids();
}

void Game::UpdateInput() {
	// Switch mouse mode
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_M)) {
		if (g_theInput->GetMouseMode() == MOUSEMODE_FREE) {
			g_theInput->SetMouseMode(MOUSEMODE_SNAP);
		}
		else {
			g_theInput->SetMouseMode(MOUSEMODE_FREE);
		}
	}

	float ds = m_gameClock->GetDeltaSeconds();

	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_I)) {
		ds *= 10.f;
	}
	else if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_O)) {
		ds *= 0.1f;
	}

	if (g_theInput->WasMouseJustScrolledDown()) {
		float cameraHeight = m_mainCamera2D->GetOrtho().GetDimensions().y;
		cameraHeight -= 10.f;
		float cameraWidth = cameraHeight * (16.f / 9.f);
		Vector2 cameraCenter = m_mainCamera2D->GetOrtho().GetCenter();
		m_mainCamera2D->SetOrtho(cameraCenter - 0.5f * Vector2(cameraWidth, cameraHeight), cameraCenter + 0.5f * Vector2(cameraWidth, cameraHeight), 0.f, 10.f);
	}
	if (g_theInput->WasMouseJustScrolledUp()) {
		float cameraHeight = m_mainCamera2D->GetOrtho().GetDimensions().y;
		cameraHeight += 10.f;
		float cameraWidth = cameraHeight * (16.f / 9.f);
		Vector2 cameraCenter = m_mainCamera2D->GetOrtho().GetCenter();
		m_mainCamera2D->SetOrtho(cameraCenter - 0.5f * Vector2(cameraWidth, cameraHeight), cameraCenter + 0.5f * Vector2(cameraWidth, cameraHeight), 0.f, 10.f);
	}
}

void Game::DebugDrawGrids() const {


	int minX = RoundToNearestInt(m_mainCamera2D->GetOrtho().mins.x);
	int maxX = RoundToNearestInt(m_mainCamera2D->GetOrtho().maxs.x);
	int minY = RoundToNearestInt(m_mainCamera2D->GetOrtho().mins.y);
	int maxY = RoundToNearestInt(m_mainCamera2D->GetOrtho().maxs.y);

	for (int x = minX; x <= maxX; ++x) {
		g_theRHI->GetImmediateRenderer()->DrawLine2D(Vector2(x, minY), Vector2(x, maxY), Rgba::GRAY);
	}

	g_theRHI->GetFontRenderer()->SetSize(10.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);

	g_theRHI->GetFontRenderer()->DrawTextInBox(Stringf("%d, %d", (int)m_mainCamera2D->GetOrtho().mins.x, (int)m_mainCamera2D->GetOrtho().maxs.x),
		AABB2(Vector2(0.f, g_mainOutput->GetHeight() - 32.f), Vector2(g_mainOutput->GetWidth(), g_mainOutput->GetHeight())));
}
