#include "Engine/Core/Camera.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
#include "Game/ComposerGame.hpp"
#include "Game/TheApp.hpp"

ComposerGame::ComposerGame() {
	m_midiPlayer = std::make_unique<MIDIPlayer>();

	m_gameClock = std::make_unique<Clock>(g_theMasterClock.get());
	g_theMasterClock->AddChild(m_gameClock.get());

	// Create the Board window camera
	m_boardCamera = std::make_unique<Camera>();
	m_boardCamera->SetViewport(0U, 0, 0, g_boardOutput->GetWidth(), g_boardOutput->GetHeight(), 0.f, 1.f);
	m_boardCamera->SetProjectionMode(ORTHOGRAPHIC);
	m_boardCamera->SetOrtho(Vector2::ZERO, Vector2(g_boardOutput->GetWidth(), g_boardOutput->GetHeight()), 0.f, 10.f);

	// Setup debug draw system to use my 2d/3d cameras
	g_theDebugDrawSystem->SetCamera2D(m_boardCamera.get());

	m_board = std::make_unique<Board>();

	LoadDefaultResources();

	//m_midiPlayer->SwitchInstrument(101);
	m_midiPlayer->SwitchToDrums();
}

ComposerGame::~ComposerGame() {

}

void ComposerGame::LoadDefaultResources() {
	g_theResourceManager->LoadSampler("linear");
	g_theResourceManager->LoadSampler("nearest");
	g_theResourceManager->LoadTexture2D("default", "Data/Images/white.png");
	g_theResourceManager->LoadTexture2D("terrain_test", "Data/Images/terrain_test.png");
	g_theResourceManager->LoadTexture2D("terrain_diffuse", "Data/Images/terrain_diffuse.png");
	g_theResourceManager->LoadTexture2D("terrain_normal", "Data/Images/terrain_normal.png");
	g_theResourceManager->LoadTexture2D("test_opengl", "Data/Images/test_opengl.png");
	g_theResourceManager->LoadSpriteSheet("terrain_32x32", "Data/Images/terrain_32x32.png", IntVector2(32, 32));
	g_theResourceManager->LoadShaderProgram("default_vs", "Data/Shaders/hlsl/default_vs.hlsl", SHADER_TYPE_VERTEX_SHADER);
	g_theResourceManager->LoadShaderProgram("default_ps", "Data/Shaders/hlsl/default_ps.hlsl", SHADER_TYPE_PIXEL_SHADER);
	g_theResourceManager->LoadMaterial("default", "Data/Materials/default.mat");
	g_theResourceManager->LoadMaterial("terrain", "Data/Materials/terrain.mat");
}

Vector2 ComposerGame::GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const {
	return Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (float)gridCoords.x, GRID_HEIGHT_IN_BOARD_WINDOW * (float)gridCoords.y);
}

Vector2 ComposerGame::GetGridCenterFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const {
	return GetGridPositionFromGridCoordsOnBoardWindow(gridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
}

void ComposerGame::Update() {
	if (!m_board->m_isReady) {
		return;
	}

	float bps = (float)m_tempo / 60.f;
	float prevIndicatorPos = m_currentIndicatorPos;
	m_currentIndicatorPos += bps * m_gameClock->GetDeltaSeconds();
	
	if (m_currentIndicatorPos >= 38.f) {
		m_currentIndicatorPos -= 36.f;
	}

	int prevIndicatorIdx = floorf(prevIndicatorPos) - 2;
	int curIndicatorIdx = floorf(m_currentIndicatorPos) - 2;

	if (prevIndicatorIdx != curIndicatorIdx) {
		m_pianoTrack[curIndicatorIdx].clear();
		std::fill(m_drumTrack.begin(), m_drumTrack.end(), false);

		BoardData_t* data = m_board->m_boardData.Read();
		if (data) {
			for (int y = 0; y < 23; ++y) {
				int gridIndex = y * GRID_COUNT_X + curIndicatorIdx + 2;
				if (data->greenFractions[gridIndex] >= 0.8f) {
					m_pianoTrack[curIndicatorIdx].push_back(m_notes[y]);
				}
				else {
					if (data->blueFractions[gridIndex] >= 0.8f) {
						m_drumTrack[curIndicatorIdx] = true;
					}
				}
			}
		}

		const std::vector<u8> notes = m_pianoTrack[curIndicatorIdx];
		for (auto n : notes) {
			if (n != 0) {
				m_midiPlayer->PlayNote(n, 50);
			}
		}
		if (m_drumTrack[curIndicatorIdx] == true) {
			m_midiPlayer->PlayDrums(); 
		}
	}

	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_I)) {
		m_tempo++;
	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_O)) {
		m_tempo--;
	}
}

void ComposerGame::Render() const {
	if (m_board->m_isReady == false) {
		//-------------------------------Guard------------------------------
		ClearBoardWindow();
		RenderInitialQuadsAtCorner();
	}
	else {
		ClearBoardWindow();
		RenderGridLinesOnBoardWindow();
		RenderTempoIndicator();
		RenderNotesName();
	}
}

void ComposerGame::ClearBoardWindow() const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());

	g_theRHI->GetDevice()->ClearColor(m_boardCamera->GetRTV(), Rgba::WHITE);
	g_theRHI->GetDevice()->ClearDepthStencil(m_boardCamera->GetDSV(), 1.f, 0U);
}

void ComposerGame::RenderGridLinesOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);

	Mesh<VertexPCU> lines(PRIMITIVE_TYPE_LINELIST, false);
	for (int x = 2; x <= 38; ++x) {
		Vector2 startPos((float)x * GRID_WIDTH_IN_BOARD_WINDOW, 0.f);
		Vector2 endPos((float)x * GRID_WIDTH_IN_BOARD_WINDOW, 23.f * GRID_HEIGHT_IN_BOARD_WINDOW);
		lines.AddLine(startPos, endPos, Rgba::BLACK);
	}
	for (int y = 0; y <= 22; ++y) {   
		Vector2 startPos(2.f * GRID_WIDTH_IN_BOARD_WINDOW, (float)y * GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 endPos(38.f * GRID_WIDTH_IN_BOARD_WINDOW, (float)y * GRID_HEIGHT_IN_BOARD_WINDOW);
		lines.AddLine(startPos, endPos, Rgba::BLACK);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&lines);
}

void ComposerGame::RenderInitialQuadsAtCorner() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	// Draw alignment quads
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW), Vector2(1.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, HEIGHT_IN_BOARD_WINDOW), Vector2(0.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, 0.f), Vector2(1.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
}

void ComposerGame::RenderTempoIndicator() const {
	Vector2 pos(m_currentIndicatorPos * GRID_WIDTH_IN_BOARD_WINDOW, 0.f);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(5.f, HEIGHT_IN_BOARD_WINDOW), Rgba::PURPLE);
}

void ComposerGame::RenderNotesName() const {
	g_theRHI->GetFontRenderer()->SetSize(32.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);

	for (int y = 0; y < 23; ++y) {
		Vector2 center = GetGridCenterFromGridCoordsOnBoardWindow(IntVector2(2, y));
		AABB2 box(center, GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
		g_theRHI->GetFontRenderer()->DrawTextInBox(m_noteNames[y], box, Rgba::BLACK);
	}
}
