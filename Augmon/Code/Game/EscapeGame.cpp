#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Game/EscapeGame.hpp"
#include "Game/SetupState.hpp"
#include "Game/VictoryState.hpp"
#include "Game/EncounterState.hpp"
#include "Game/EscapeWorld.hpp"
#include "Game/TheApp.hpp"

std::vector<Task_t> g_tasks;

EscapeGame::EscapeGame() {
	m_world = std::make_unique<EscapeWorld>();
	RebuildTileMesh();

	m_gameClock = std::make_unique<Clock>(g_theMasterClock.get());
	g_theMasterClock->AddChild(m_gameClock.get());

	AddGameState("Setup");
	AddGameState("Encounter");
	AddGameState("Victory");

	ChangeToState("Setup");
}

EscapeGame::~EscapeGame() {

}

void EscapeGame::Update() {
	PROFILE_LOG_SCOPE("Game Update");

	float deltaSeconds = m_gameClock->GetDeltaSeconds();

	if (m_currentState != m_nextState) {
		if (m_currentState) {
			m_currentState->OnExit();
		}
		m_currentState = m_nextState;
		m_currentState->OnEnter();
	}
	m_currentState->Update(deltaSeconds);
}

void EscapeGame::Render() const {
	PROFILE_LOG_SCOPE("Game Render");

	g_theRHI->GetFontRenderer()->SetFont(L"Showcard Gothic");
	m_currentState->Render();

	//--------------------------------------------------------------------
	for (auto it = g_tasks.begin(); it != g_tasks.end(); ) {
		if (!it->m_stopWatch->HasElapsed()) {
			it->Execute();
			++it;
		}
		else {
			it = g_tasks.erase(it);
		}
	}
}

void EscapeGame::AddGameState(const std::string& name) {
	if (name == "Setup") {
		Uptr<GameState> newState = std::make_unique<SetupState>(this, m_world.get());
		m_gameStates.insert({ name, std::move(newState) });
	}
	else if (name == "Victory") {
		Uptr<GameState> newState = std::make_unique<VictoryState>(this, m_world.get());
		m_gameStates.insert({ name, std::move(newState) });
	}
	else if (name == "Encounter") {
		Uptr<GameState> newState = std::make_unique<EncounterState>(this, m_world.get());
		m_gameStates.insert({ name, std::move(newState) });
	}
}

void EscapeGame::ChangeToState(const std::string& name) {
	if (m_gameStates.find(name) == m_gameStates.end()) {
		ERROR_AND_DIE("Cannot find game state!");
	}
	m_nextState = m_gameStates.at(name).get();
}

IntVector2 EscapeGame::GetMouseCursorCoordsInThiefWindow() const {
	Vector2 mouseCursorPos = g_theInput->GetMouseClientPos(g_thiefOutput->m_window->GetHandle());
	mouseCursorPos.x = ClampFloat(mouseCursorPos.x, 0.f, g_thiefOutput->GetWidth() - 1);
	mouseCursorPos.y = ClampFloat(mouseCursorPos.y, 0.f, g_thiefOutput->GetHeight() - 1);

	IntVector2 mouseCursorCoords;
	mouseCursorCoords.x = (int)(mouseCursorPos.x / m_world->m_gridWidthInThiefWindow);
	float height = g_thiefOutput->GetHeight();
	mouseCursorCoords.y = (int)((height - mouseCursorPos.y) / m_world->m_gridHeightInThiefWindow);

	return mouseCursorCoords;
}

void EscapeGame::ClearThiefWindow() {
	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
	m_world->m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
	m_world->m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_thiefCamera.get());

	g_theRHI->GetDevice()->ClearColor(m_world->m_thiefCamera->GetRTV(), Rgba::BLACK);
	g_theRHI->GetDevice()->ClearDepthStencil(m_world->m_thiefCamera->GetDSV(), 1.f, 0U);
}

void EscapeGame::ClearBoardWindow() {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	m_world->m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
	m_world->m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	g_theRHI->GetDevice()->ClearColor(m_world->m_boardCamera->GetRTV(), Rgba::BLACK);
	g_theRHI->GetDevice()->ClearDepthStencil(m_world->m_boardCamera->GetDSV(), 1.f, 0U);
}

void EscapeGame::RebuildTileMesh() {
	m_tileSet = g_theResourceManager->GetSpriteSheet("terrain_32x32");
	if (!m_tileMeshOnBoard) {
		m_tileMeshOnBoard = std::make_unique<Mesh<VertexPCU>>(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}
	else {
		m_tileMeshOnBoard->Reset(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}

	if (!m_tileMeshOnThief) {
		m_tileMeshOnThief = std::make_unique<Mesh<VertexPCU>>(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}
	else {
		m_tileMeshOnThief->Reset(PRIMITIVE_TYPE_TRIANGLELIST, true);
	}

	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
		int index = m_world->m_tiles[i].m_indexInTileset;
		// empty
		if (index == 0) {
			continue;
		}

		AABB2 uv = m_tileSet->GetUVFromIndex(index);
		int gridY = i / GRID_COUNT_X;
		int gridX = i % GRID_COUNT_X;
		Vector2 positionOnBoard(GRID_WIDTH_IN_BOARD_WINDOW * gridX, GRID_HEIGHT_IN_BOARD_WINDOW * gridY);
		Vector2 positionOnThief(m_world->m_gridWidthInThiefWindow * gridX, m_world->m_gridWidthInThiefWindow * gridY);

		m_tileMeshOnBoard->AddQuad(positionOnBoard, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, uv);
		m_tileMeshOnThief->AddQuad(positionOnThief, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, uv);
	}
}

void EscapeGame::RenderMapOnThiefWindow() const{
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(m_tileMeshOnThief.get());
}

void EscapeGame::RenderMapOnBoardWindow() const{
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(m_tileMeshOnBoard.get());
}

void EscapeGame::RenderGridLinesOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);

	Mesh<VertexPCU> lines(PRIMITIVE_TYPE_LINELIST, false);
	for (int x = (int)EscapeWorld::s_playableAreaBounds.mins.x; x <= (int)EscapeWorld::s_playableAreaBounds.maxs.x; ++x) {
		Vector2 startPos((float)x * GRID_WIDTH_IN_BOARD_WINDOW, 0.f);
		Vector2 endPos((float)x * GRID_WIDTH_IN_BOARD_WINDOW, EscapeWorld::s_playableAreaBounds.maxs.y * GRID_HEIGHT_IN_BOARD_WINDOW);
		lines.AddLine(startPos, endPos, Rgba::BLACK);
	}
	for (int y = EscapeWorld::s_playableAreaBounds.mins.y; y <= EscapeWorld::s_playableAreaBounds.maxs.y; ++y) {
		Vector2 startPos(EscapeWorld::s_playableAreaBounds.mins.x * GRID_WIDTH_IN_BOARD_WINDOW, (float)y * GRID_HEIGHT_IN_BOARD_WINDOW);
		Vector2 endPos(WIDTH_IN_BOARD_WINDOW, (float)y * GRID_HEIGHT_IN_BOARD_WINDOW);
		lines.AddLine(startPos, endPos, Rgba::BLACK);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&lines);
}

void EscapeGame::WriteTextInBox(const std::string& text, float size, eTextAlignment horizontalAlign, eTextAlignment verticalAlign, const AABB2& box, const Rgba& color) const {
	g_theRHI->GetFontRenderer()->SetSize(size);
	g_theRHI->GetFontRenderer()->SetAlignment(horizontalAlign, verticalAlign);
	g_theRHI->GetFontRenderer()->DrawTextInBox(text, box, color);
}

void EscapeGame::RenderInitialQuardsAtCorner() const{
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	// Draw alignment quads
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW), Vector2(1.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, HEIGHT_IN_BOARD_WINDOW), Vector2(0.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, 0.f), Vector2(1.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
}

void EscapeGame::RenderAvailavleTilesForSetupOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
		if (m_world->m_tiles[i].m_indexInTileset == 30) {
			int gridX = i % GRID_COUNT_X;
			int gridY = i / GRID_COUNT_X;
			if (m_world->m_treasures.find(IntVector2(gridX, gridY)) == m_world->m_treasures.end()) {
				Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(gridX, gridY));
				mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba::WHITE);
			}
		}
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::ShowAvailableTilesForThiefSetupOnThiefWindow() const {
	static bool direction = true;
	static float fraction = 0.f;
	if (fraction > 1.f) {
		direction = false;
	}
	if (fraction < 0.f) {
		direction = true;
	}
	if (direction == true) {
		fraction += m_gameClock->GetDeltaSeconds();
	}
	else {
		fraction -= m_gameClock->GetDeltaSeconds();
	}

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
		bool isEntrance = m_world->m_tiles[i].m_isEntrance;
		if (isEntrance) {
			int gridX = i % GRID_COUNT_X;
			int gridY = i / GRID_COUNT_X;
			Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(IntVector2(gridX, gridY));
			mesh.AddQuad(pos, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, Rgba(1.f, 1.f, 1.f, fraction));
		}
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderThiefCursorOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Vector2 cursorPos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(m_world->m_thief.m_cursorGridCoords);
	g_theRHI->GetImmediateRenderer()->DrawLineBox2D(AABB2(cursorPos, cursorPos + Vector2(m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow)), Rgba::RED);
}

void EscapeGame::RenderThiefSpriteOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Vector2 thiefPos = m_world->m_thief.m_centerOnThiefWindow;
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(thiefPos, Vector2(.5f, .5f), Vector2(m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow), Rgba::RED);
}

void EscapeGame::RenderGuardSpriteOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (int i = 0; i < m_world->m_guards.size(); ++i) {
		Guard* guard = &m_world->m_guards[i];
		Vector2 center = m_world->GetGridCenterFromGridCoordsOnThiefWindow(guard->m_gridCoords);
		if (guard->m_isFlashlightOn) {
			g_theRHI->GetImmediateRenderer()->DrawDashedCircle2D(center, (float)Guard::s_alertDistanceWithFlashlight * GRID_WIDTH_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 0.f, 0.5f));
		}
		else {
			g_theRHI->GetImmediateRenderer()->DrawDashedCircle2D(center, (float)Guard::s_alertDistanceDefault * GRID_WIDTH_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 0.f, 0.5f));
		}

		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(guard->m_gridCoords);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, Rgba::BLUE);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);

}

void EscapeGame::RenderAllTreasuresOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (auto& t : m_world->m_treasures) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(t.first);

		bool isOpen = t.second;
		AABB2 uv;
		if (isOpen) {
			uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_treasureOpenSpriteCoords);
		}
		else {
			uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_treasureSpriteCoords);
		}
		mesh.AddQuad(pos, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, uv);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderKnownTreasuresOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (auto& t : m_world->m_treasuresGuardKnows) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(t.first);

		bool isOpen = t.second;
		AABB2 uv;
		if (isOpen) {
			uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_treasureOpenSpriteCoords);
		}
		else {
			uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_treasureSpriteCoords);
		}
		mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, uv);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderInfoAreaBackgroundOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.mins.y), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.GetDimensions().x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_infoAreaBounds.GetDimensions().y), Rgba(1.f, 1.f, 0.f, 0.3f));
}

void EscapeGame::RenderActionAreaBackgroundOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_actionAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_actionAreaBounds.mins.y), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * EscapeWorld::s_actionAreaBounds.GetDimensions().x, GRID_HEIGHT_IN_BOARD_WINDOW * EscapeWorld::s_actionAreaBounds.GetDimensions().y), Rgba(0.f, 1.f, 1.f, 0.3f));
}

void EscapeGame::ShowAvailableTilesOnThiefWindow(const std::vector<IntVector2>& tiles) const{
	static bool direction = true;
	static float fraction = 0.f;
	if (fraction > 1.f) {
		direction = false;
	}
	if (fraction < 0.f) {
		direction = true;
	}
	if (direction == true) {
		fraction += m_gameClock->GetDeltaSeconds();
	}
	else {
		fraction -= m_gameClock->GetDeltaSeconds();
	}

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	for (auto tile : tiles) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(tile);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, Rgba(1.f, 1.f, 1.f, fraction));
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::ShowAvailableTilesOnBoardWindow(const std::vector<IntVector2>& tiles) const {
	static bool direction = true;
	static float fraction = 0.f;
	if (fraction > 1.f) {
		direction = false;
	}
	if (fraction < 0.f) {
		direction = true;
	}
	if (direction == true) {
		fraction += m_gameClock->GetDeltaSeconds();
	}
	else {
		fraction -= m_gameClock->GetDeltaSeconds();
	}

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	for (auto tile : tiles) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(tile);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 1.f, fraction));
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderThiefMovePathOnThiefWindow(const std::deque<IntVector2>& gridCoords) const {
	if (gridCoords.empty()) {
		return;
	}
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, false);

	for (int i = 0; i < gridCoords.size(); ++i) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(gridCoords[i]);
		pos += 0.5f * Vector2(m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow);
		mesh.AddDisc2D(pos, m_world->m_gridWidthInThiefWindow * 0.5f - 5.f, Rgba::RED);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);

	Mesh<VertexPCU> lines(PRIMITIVE_TYPE_LINELIST, false);
	for (int i = 0; i < gridCoords.size() - 1; ++i) {
		Vector2 curPos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(gridCoords[i]);
		curPos += 0.5f * Vector2(m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow);

		Vector2 nextPos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(gridCoords[i + 1]);
		nextPos += 0.5f * Vector2(m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow);

		lines.AddLine(curPos, nextPos, Rgba::RED);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&lines);

}

void EscapeGame::RenderWhiteTilesForGuardsOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	for (int i = 0; i < m_world->m_guards.size(); ++i) {
		Guard* guard = &m_world->m_guards[i];
		Vector2 pos = m_world->GetGridCenterFromGridCoordsOnBoardWindow(guard->m_gridCoords);
		mesh.AddQuad(pos, Vector2(0.5f, 0.5f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba::WHITE);

		if (guard->m_isFlashlightOn) {
			float radius = Vector2(0.5f * GRID_WIDTH_IN_BOARD_WINDOW, 0.8f * GRID_HEIGHT_IN_BOARD_WINDOW).GetLength();
			for (int idx = 0; idx < 8; ++idx) {
				float theta = (float)idx * 45.f;
				Vector2 disp = PolarToCartesian(radius, theta);
				Vector2 startPos = pos + disp;
				Vector2 endPos = pos + 1.5f * disp;
				g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos, endPos, Rgba::YELLOW);
			}
		}
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::ShowCursorForGuardOnBoardWindow(int guardIdx) const {
	static bool direction = true;
	static float fraction = 0.f;
	if (fraction > 1.f) {
		direction = false;
	}
	if (fraction < 0.f) {
		direction = true;
	}
	if (direction == true) {
		fraction += m_gameClock->GetDeltaSeconds();
	}
	else {
		fraction -= m_gameClock->GetDeltaSeconds();
	}

	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_guardCursorSpriteCoords);
	Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(m_world->m_guards[guardIdx].m_gridCoords);
	mesh.AddQuad(pos + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Vector2(0.5f, 0.5f), GRID_WIDTH_IN_BOARD_WINDOW + 5.f + fraction * 20.f, GRID_HEIGHT_IN_BOARD_WINDOW + 5.f + fraction * 20.f, uv, Rgba::GREEN);
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderActionItemSlotOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	for (int x = EscapeWorld::s_actionItemSlotBounds.mins.x; x < EscapeWorld::s_actionItemSlotBounds.maxs.x; ++x) {
		for (int y = EscapeWorld::s_actionItemSlotBounds.mins.y; y < EscapeWorld::s_actionItemSlotBounds.maxs.y; ++y) {
			Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(x, y));
			mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba::WHITE);
		}
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);

	Vector2 boxMins = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_actionItemSlotBounds.maxs.y));
	Vector2 boxMaxs = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(4, EscapeWorld::s_actionItemSlotBounds.maxs.y + 2));

	WriteTextInBox("Action Item Slot", 24.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(boxMins, boxMaxs), Rgba::CYAN);
}

void EscapeGame::RenderEquipmentSlotOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	for (int x = EscapeWorld::s_equipmentSlotBounds.mins.x; x < EscapeWorld::s_equipmentSlotBounds.maxs.x; ++x) {
		for (int y = EscapeWorld::s_equipmentSlotBounds.mins.y; y < EscapeWorld::s_equipmentSlotBounds.maxs.y; ++y) {
			Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(x, y));
			mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba::WHITE);
		}
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);

	Vector2 boxMins = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(0, EscapeWorld::s_equipmentSlotBounds.maxs.y));
	Vector2 boxMaxs = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(4, EscapeWorld::s_equipmentSlotBounds.maxs.y + 2));

	WriteTextInBox("Equipment Slot", 24.f, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER, AABB2(boxMins, boxMaxs), Rgba::CYAN);
}

void EscapeGame::RenderActionPointOnBoardWindow(float isOn) const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(EscapeWorld::s_actionItemSlotBounds.maxs.GetAsIntVector2()) + Vector2(GRID_WIDTH_IN_BOARD_WINDOW * 0.5f, 0.f);
	Rgba color = isOn ? Rgba::GREEN : Rgba::RED;
	g_theRHI->GetImmediateRenderer()->DrawDisc2D(pos, GRID_WIDTH_IN_BOARD_WINDOW * 0.3f, color);
}

void EscapeGame::RenderEquipmentPointOnBoardWindow(float isOn) const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(EscapeWorld::s_equipmentSlotBounds.maxs.GetAsIntVector2()) + Vector2(GRID_WIDTH_IN_BOARD_WINDOW * 0.5f, 0.f);
	Rgba color = isOn ? Rgba::GREEN : Rgba::RED;
	g_theRHI->GetImmediateRenderer()->DrawDisc2D(pos, GRID_WIDTH_IN_BOARD_WINDOW * 0.3f, color);
}

void EscapeGame::RenderActionItemPatternOnBoardWindow(eActionType actionItem, const IntVector2& gridCoords, float width) {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
	Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(gridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

	switch (actionItem) {
	case ACTION_TYPE_SINGLEMONITOR:
		mesh.AddQuad(pos, Vector2(1.f, 0.f), width * 0.25f, width * 0.25f, Rgba::GREEN);
		mesh.AddQuad(pos, Vector2(0.f, 0.f), width * 0.25f, width * 0.25f, Rgba::GREEN);
		mesh.AddQuad(pos, Vector2(1.f, 1.f), width * 0.25f, width * 0.25f,Rgba::GREEN);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), width * 0.25f, width * 0.25f,Rgba::RED);
		break;
	case ACTION_TYPE_SCANALLMONITORS:
		mesh.AddQuad(pos, Vector2(1.f, 0.f), width * 0.25f, width * 0.25f, Rgba::RED);
		mesh.AddQuad(pos, Vector2(0.f, 0.f), width * 0.25f, width * 0.25f, Rgba::RED);
		mesh.AddQuad(pos, Vector2(1.f, 1.f), width * 0.25f, width * 0.25f, Rgba::RED);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), width * 0.25f, width * 0.25f, Rgba::RED);
		break;
	case ACTION_TYPE_MOTIONDETECTOR:
		mesh.AddQuad(pos, Vector2(1.f, 0.f), width * 0.25f, width * 0.25f, Rgba::YELLOW);
		mesh.AddQuad(pos, Vector2(0.f, 0.f), width * 0.25f, width * 0.25f, Rgba::YELLOW);
		mesh.AddQuad(pos, Vector2(1.f, 1.f), width * 0.25f, width * 0.25f, Rgba::GREEN);
		mesh.AddQuad(pos, Vector2(0.f, 1.f), width * 0.25f, width * 0.25f, Rgba::GREEN);
		break;
	default:
		break;
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderAllMonitorsOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (auto& t : m_world->m_monitors) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnThiefWindow(t.first);

		bool isActive = t.second;
		AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(EscapeWorld::s_monitorSpriteCoords);
		Rgba color;
		if (isActive) {
			color = Rgba::WHITE;
		}
		else {
			color = Rgba::RED;
		}
		mesh.AddQuad(pos, Vector2(0.f, 1.f), m_world->m_gridWidthInThiefWindow, m_world->m_gridHeightInThiefWindow, uv, color);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	// Render monitor sight on Thief Window
	for (auto m : m_world->m_monitors) {
		if (m.second) {
			Vector2 center = m_world->GetGridCenterFromGridCoordsOnThiefWindow(m.first);
			g_theRHI->GetImmediateRenderer()->DrawDashedCircle2D(center, (float)m_world->m_monitorAlertDistanceDefault * m_world->m_gridWidthInThiefWindow, Rgba(1.f, 1.f, 0.f, 0.5f));

			RaycastResult2D_t resultLeft = m_world->RaycastOnThiefWindow(center, Vector2(-1.f, 0.f), (float)GRID_COUNT_X * m_world->m_gridWidthInThiefWindow);
			RaycastResult2D_t resultRight = m_world->RaycastOnThiefWindow(center, Vector2(1.f, 0.f), (float)GRID_COUNT_X * m_world->m_gridWidthInThiefWindow);
			RaycastResult2D_t resultUp = m_world->RaycastOnThiefWindow(center, Vector2(0.f, 1.f), (float)GRID_COUNT_Y * m_world->m_gridHeightInThiefWindow);
			RaycastResult2D_t resultDown = m_world->RaycastOnThiefWindow(center, Vector2(0.f, -1.f), (float)GRID_COUNT_Y * m_world->m_gridHeightInThiefWindow);

			if (resultLeft.m_didImpact) {
				g_theRHI->GetImmediateRenderer()->DrawLine2D(center, resultLeft.m_impactPos, Rgba(1.f, 1.f, 0.f, 0.5f));
			}
			if (resultRight.m_didImpact) {
				g_theRHI->GetImmediateRenderer()->DrawLine2D(center, resultRight.m_impactPos, Rgba(1.f, 1.f, 0.f, 0.5f));
			}
			if (resultUp.m_didImpact) {
				g_theRHI->GetImmediateRenderer()->DrawLine2D(center, resultUp.m_impactPos, Rgba(1.f, 1.f, 0.f, 0.5f));
			}
			if (resultDown.m_didImpact) {
				g_theRHI->GetImmediateRenderer()->DrawLine2D(center, resultDown.m_impactPos, Rgba(1.f, 1.f, 0.f, 0.5f));
			}
		}
	}
}

void EscapeGame::RenderKnownMonitorsOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (auto& t : m_world->m_monitorsGuardKnows) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(t.first);

		bool isActive = t.second;
		Rgba color;
		if (isActive) {
			color = Rgba::WHITE;
		}
		else {
			color = Rgba::RED;
		}
		mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, color);
	}
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::ShowAvailableTilesForKnownMonitorsOnBoard() const {
	static bool direction = true;
	static float fraction = 0.f;
	if (fraction > 1.f) {
		direction = false;
	}
	if (fraction < 0.f) {
		direction = true;
	}
	if (direction == true) {
		fraction += m_gameClock->GetDeltaSeconds();
	}
	else {
		fraction -= m_gameClock->GetDeltaSeconds();
	}

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	for (auto m : m_world->m_monitorsGuardKnows) {
		Vector2 pos = m_world->GetGridPositionFromGridCoordsOnBoardWindow(m.first);

		if (m.second) {
			mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 1.f, fraction));
		}
		else {
			mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba::RED);
		}
	}

	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}

void EscapeGame::RenderThiefSpriteOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	Mesh<VertexPCU> mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);

	float norm = Clamp01(m_world->m_thief.m_fractionOfExposion + 0.2f);
	Rgba color = Rgba(1.f, 0.f, 0.f, norm);

	mesh.AddQuad(m_world->m_thief.m_centerOnBoardWindow, Vector2(0.5f, 0.5f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, color);
	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
}


void EscapeGame::RenderBatteryOnBoardWindow(int value) const {
	Vector2 boxMins = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(EscapeWorld::s_equipmentSlotBounds.mins.x, EscapeWorld::s_equipmentSlotBounds.mins.y - 3));
	Vector2 boxMaxs = m_world->GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(EscapeWorld::s_equipmentSlotBounds.maxs.x, EscapeWorld::s_equipmentSlotBounds.mins.y - 2));

	float fraction = (float)value / (float)Guard::s_flashlightFullBattery;

	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);

	Rgba startColor = Rgba::RED;
	Rgba endColor = Rgba::GREEN;
	Rgba batteryColor = Interpolate(startColor, endColor, fraction);

	g_theRHI->GetImmediateRenderer()->DrawQuad2D(boxMins, Vector2(0.f, 1.f), Vector2(2.f * fraction * GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), batteryColor);
	g_theRHI->GetImmediateRenderer()->DrawLineBox2D(AABB2(boxMins, boxMaxs));

	g_theRHI->GetFontRenderer()->SetSize(30.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox(Stringf("%d", value), AABB2(boxMins, boxMaxs), Rgba(255 - batteryColor.r, 255 - batteryColor.g, 255 - batteryColor.b));
}

void EscapeGame::RenderThiefWinOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, 0.f), Vector2(0.f, 1.f), Vector2(m_world->m_widthInThiefWindow, m_world->m_heightInThiefWindow), Rgba(1.f, 0.f, 0.f, 0.5f));

	g_theRHI->GetFontRenderer()->SetSize(50.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief Wins!", m_world->m_thiefCamera->GetOrtho(), Rgba::YELLOW);
}

void EscapeGame::RenderThiefWinOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, 0.f), Vector2(0.f, 1.f), Vector2(WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW), Rgba(1.f, 0.f, 0.f, 0.5f));

	g_theRHI->GetFontRenderer()->SetSize(50.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief Wins!", m_world->m_boardCamera->GetOrtho(), Rgba::YELLOW);
}

void EscapeGame::RenderGuardWinOnThiefWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, 0.f), Vector2(0.f, 1.f), Vector2(m_world->m_widthInThiefWindow, m_world->m_heightInThiefWindow), Rgba(0.f, 0.f, 1.f, 0.5f));

	g_theRHI->GetFontRenderer()->SetSize(50.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Guard Wins!", m_world->m_thiefCamera->GetOrtho(), Rgba::YELLOW);
}

void EscapeGame::RenderGuardWinOnBoardWindow() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, 0.f), Vector2(0.f, 1.f), Vector2(WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW), Rgba(0.f, 0.f, 1.f, 0.5f));

	g_theRHI->GetFontRenderer()->SetSize(50.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Guard Wins!", m_world->m_boardCamera->GetOrtho(), Rgba::YELLOW);
}

void EscapeGame::ShowThiefIsRevealedOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief is revealed!", bounds, Rgba::RED);
}

void EscapeGame::ShowClearOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Clear!", bounds, Rgba::GREEN);
}

void EscapeGame::ShowPickUpTreasureOnThiefWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_thiefCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Pick up treasure!", bounds, Rgba::GREEN);
}

void EscapeGame::ShowDeactivateMonitorOnThiefWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_thiefCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Deactivate Monitor!", bounds, Rgba::GREEN);
}

void EscapeGame::ShowLockedOnThiefWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_thiefCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Locked!", bounds, Rgba::YELLOW);
}

void EscapeGame::ShowTreasureIsStolenOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Treasure Was Stolen!", bounds, Rgba::YELLOW);
}

void EscapeGame::ShowMonitorWasDeactivatedOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Monitor Was Deactivated!", bounds, Rgba::YELLOW);
}

void EscapeGame::ShowMonitorWasRecoveredOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Monitor Was Recovered!", bounds, Rgba::GREEN);
}

void EscapeGame::ShowNotEnoughTreasuresOnBoardWindow(float normalizedTime, const Vector2& pos) const {
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
	g_theRHI->GetImmediateRenderer()->BindCamera(m_world->m_boardCamera.get());

	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);

	g_theRHI->GetFontRenderer()->SetSize(24.f);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
	g_theRHI->GetFontRenderer()->DrawTextInBox("Need At Least 3 Treasures!", bounds, Rgba::YELLOW);
}

