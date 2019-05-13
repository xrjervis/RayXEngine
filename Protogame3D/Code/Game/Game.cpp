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
#include "Game/Game.hpp"
#include "Game/TheApp.hpp"

Game::Game() {
//	//--------------------------------------------------------------------------
//	// Set game coords matrix
// 	Matrix44::GameToEngine = Matrix44(Vector3(0.f, 0.f, 1.f), Vector3(-1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
// 	Matrix44::EngineToGame = Matrix44::GameToEngine;
// 	Matrix44::EngineToGame.Inverse();

	//--------------------------------------------------------------------------
	// Create my game clock
	m_gameClock = std::make_unique<Clock>(g_theMasterClock.get());
	g_theMasterClock->AddChild(m_gameClock.get());

	//--------------------------------------------------------------------------
	// Create the main 2d camera
	m_mainCamera2D = std::make_unique<Camera>();
	m_mainCamera2D->SetViewport(0U, 0, 0, g_mainOutput->GetWidth(), g_mainOutput->GetHeight(), 0.f, 1.f);
	m_mainCamera2D->SetProjectionMode(ORTHOGRAPHIC);
	m_mainCamera2D->SetOrtho(Vector2::ZERO, Vector2(g_mainOutput->GetWidth(), g_mainOutput->GetHeight()), 0.f, 10.f);
	m_mainCamera2D->SetRenderTarget(g_mainOutput->GetRTV());
	m_mainCamera2D->SetDepthTarget(g_mainOutput->GetDSV());

	//--------------------------------------------------------------------------
	// Create the main 3d camera
	m_mainCamera3D = std::make_unique<Camera>();
	m_mainCamera3D->SetViewport(0U, 0, 0, g_mainOutput->GetWidth(), g_mainOutput->GetHeight(), 0.f, 1.f);
	m_mainCamera3D->SetProjectionMode(PERSPECTIVE);
	m_mainCamera3D->SetPerspective(45.f, 16.f / 9.f, 0.1f, 2000.f);
	m_mainCamera3D->SetRenderTarget(g_mainOutput->GetRTV());
	m_mainCamera3D->SetDepthTarget(g_mainOutput->GetDSV());

	//--------------------------------------------------------------------------
	// Setup debug draw system to use my 2d/3d cameras
	g_theDebugDrawSystem->SetCamera2D(m_mainCamera2D.get());
	g_theDebugDrawSystem->SetCamera3D(m_mainCamera3D.get());

	DebugString(10.f, "World Begin!", Rgba::GREEN, Rgba::RED);


	//--------------------------------------------------------------------
	m_midiPlayer = std::make_unique<MIDIPlayer>();
}

Game::~Game() {

}

void Game::Update() {
	float dt = m_gameClock->GetDeltaSeconds();
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_Q)) {
		m_midiPlayer->SwitchInstrument(0);
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_W)) {
		m_midiPlayer->SwitchInstrument(8);
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_E)) {
		m_midiPlayer->SwitchInstrument(24);
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_R)) {
		m_midiPlayer->SwitchInstrument(40);
	}


	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_A)) {
		m_midiPlayer->PlayNote(60);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_A)) {
		m_midiPlayer->StopNote(60);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_S)) {
		m_midiPlayer->PlayNote(62);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_S)) {
		m_midiPlayer->StopNote(62);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_D)) {
		m_midiPlayer->PlayNote(64);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_D)) {
		m_midiPlayer->StopNote(64);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_F)) {
		m_midiPlayer->PlayNote(65);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_F)) {
		m_midiPlayer->StopNote(65);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_G)) {
		m_midiPlayer->PlayNote(67);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_G)) {
		m_midiPlayer->StopNote(67);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_H)) {
		m_midiPlayer->PlayNote(69);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_H)) {
		m_midiPlayer->StopNote(69);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_J)) {
		m_midiPlayer->PlayNote(71);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_J)) {
		m_midiPlayer->StopNote(71);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_K)) {
		m_midiPlayer->PlayNote(72);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_K)) {
		m_midiPlayer->StopNote(72);
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_L)) {
		m_midiPlayer->PlayNote(74);
	}
	if (g_theInput->WasKeyJustReleased(InputSystem::KEYBOARD_L)) {
		m_midiPlayer->StopNote(74);
	}
// 	float cameraRotateSpeed;
// 	float cameraSpeed;
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_SHIFT)) {
// 		cameraRotateSpeed = 0.5f;
// 		cameraSpeed = 30.f;
// 	}
// 	else {
// 		cameraRotateSpeed = 0.2f;
// 		cameraSpeed = 5.f;
// 	}
// 
// 	Vector3 forward = m_mainCamera3D->m_transform.GetForward();
// 	forward.y = 0;
// 	forward = forward.GetNormalized();
// 	Vector3 right = m_mainCamera3D->m_transform.GetRight();
// 	Vector3 up(0.f, 1.f, 0.f);
// 
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_W)) {
// 		m_mainCamera3D->m_transform.Translate(forward * cameraSpeed * dt);
// 	}
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_S)) {
// 		m_mainCamera3D->m_transform.Translate(-forward * cameraSpeed * dt);
// 	}
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_A)) {
// 		m_mainCamera3D->m_transform.Translate(-right * cameraSpeed * dt);
// 	}
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_D)) {
// 		m_mainCamera3D->m_transform.Translate(right * cameraSpeed * dt);
// 	}
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_Q)) {
// 		m_mainCamera3D->m_transform.Translate(-up * cameraSpeed * dt);
// 	}
// 	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_E)) {
// 		m_mainCamera3D->m_transform.Translate(up * cameraSpeed * dt);
// 	}
// 	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_M)) {
// 		if (g_theInput->GetMouseMode() == MOUSEMODE_FREE) {
// 			g_theInput->SetMouseMode(MOUSEMODE_SNAP);
// 		}
// 		else {
// 			g_theInput->SetMouseMode(MOUSEMODE_FREE);
// 		}
// 	}
// 	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
// 		m_mainCamera3D->m_transform.SetWorldMatrix(Matrix44());
// 	}
// 
// 	Vector2 mouseDelta = g_theInput->GetMouseDelta();
// 	if (mouseDelta.x != 0.f) {
// 		m_mainCamera3D->m_transform.Rotate(Vector3(0.f, 1.f, 0.f) * mouseDelta.x * cameraRotateSpeed);
// 	}
// 	if (mouseDelta.y != 0.f) {
// 		m_mainCamera3D->m_transform.Rotate(Vector3(1.f, 0.f, 0.f) * mouseDelta.y * cameraRotateSpeed);
// 	}
// 	//--------------------------------------------------------------------
// 	// Print Camera Position
// 	DebugString(0.f, Stringf("Camera Position: (%.2f, %.2f, %.2f)", m_mainCamera3D->m_transform.GetWorldPosition().z, -m_mainCamera3D->m_transform.GetWorldPosition().x, m_mainCamera3D->m_transform.GetWorldPosition().y), Rgba::WHITE, Rgba::WHITE);
}

void Game::Render() const {
	//--------------------------------------------------------------------------
	// Setup render pipeline
	g_theRHI->GetFontRenderer()->BindOutput(g_mainOutput.get());
	g_theRHI->GetDevice()->ClearColor(g_mainOutput->GetRTV(), Rgba::MIDNIGHTBLUE);
	g_theRHI->GetDevice()->ClearDepthStencil(g_mainOutput->GetDSV(), 1.f, 0U);

	g_theRHI->GetImmediateRenderer()->BindCamera(m_mainCamera2D.get());
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(m_mainCamera2D->GetOrtho().GetCenter(), Vector2(0.5f, 0.5f), Vector2(50.f, 50.f), Rgba::YELLOW);

	g_theRHI->GetFontRenderer()->SetFont(L"Fira Code");
	g_theRHI->GetFontRenderer()->SetSize(32.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Hello, Protogame3D!",
		AABB2(Vector2(0.f, g_mainOutput->GetHeight() - 32.f), Vector2(g_mainOutput->GetWidth(), g_mainOutput->GetHeight())));

 	g_theRHI->GetImmediateRenderer()->BindCamera(m_mainCamera3D.get());
 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("wolf"));
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(g_theResourceManager->GetSkeletalMesh("wolf"));
}

