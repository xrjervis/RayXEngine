// #include "Engine/Core/Camera.hpp"
// #include "Engine/Core/Clock.hpp"
// #include "Engine/Core/EngineCommon.hpp"
// #include "Engine/Core/ResourceManager.hpp"
// #include "Engine/Core/Rgba.hpp"
// #include "Engine/Math/AABB2.hpp"
// #include "Engine/Renderer/d3d11/RHIInstance.hpp"
// #include "Engine/Renderer/d3d11/ImmediateRenderer.hpp"
// #include "Engine/Renderer/d3d11/FontRenderer.hpp"
// #include "Engine/Renderer/d3d11/Mesh.hpp"
// #include "Engine/InputSystem/InputSystem.hpp"
// #include "Engine/Math/MathUtils.hpp"
// #include "Engine/Audio/AudioSystem.hpp"
// #include "Game/Game.hpp"
// #include "Game/TheApp.hpp"
// 
// static Pattern_t s_guardPattern(1, 1, { Rgba::BLUE });
// static Pattern_t s_cameraPattern(1, 1, { Rgba::GREEN });
// static Pattern_t s_emptyPattern(2, 2, { Rgba::WHITE, Rgba::WHITE, Rgba::WHITE, Rgba::WHITE });
// static Pattern_t s_flashLightPattern(2, 2, { Rgba::BLUE, Rgba::BLUE, Rgba::YELLOW, Rgba::YELLOW });
// static Pattern_t s_singleCameraPattern(2, 2, { Rgba::RED, Rgba::RED, Rgba::RED, Rgba::BLUE });
// static Pattern_t s_scanAllCameraPattern(2, 2, { Rgba::RED, Rgba::RED, Rgba::RED, Rgba::RED });
// static Pattern_t s_motionDetectorPattern(2, 2, { Rgba::YELLOW, Rgba::YELLOW, Rgba::GREEN, Rgba::GREEN });
// 
// 
// static IntVector2 s_cameraSpriteCoords(23, 19);
// static IntVector2 s_unknownSpriteCoords(1, 2);
// static IntVector2 s_treasureSpriteCoords(15, 8);
// 
// 
// Game::Game() {
// 	Init();
// }
// 
// Game::~Game() {
// 
// }
// 
// void Game::Init() {
// }
// 
// void Game::Update() {
// 	m_currentState = m_nextState;
// 	switch (m_currentState) {
// 	case GAME_ATTRACT:		Update_Attract();		break;
// 	case CAMERA_SETUP:		Update_Camera_Setup();  break;
// 	case GUARD_SETUP:		Update_Guard_Setup();	break;
// 	case THIEF_SETUP:		Update_Thief_Setup();	break;
// 	case THIEF_MOVE:		Update_Thief_Move();	break;
// 	case GUARD_WAIT:		Update_Guard_Wait();	break;
// 	case GUARD_MOVE:		Update_Guard_Move();	break;
// 	case GUARD_ACT:			Update_Guard_Act();		break;
// 	case THIEF_WIN:			Update_Thief_Win();		break;
// 	case GUARD_WIN:			Update_Guard_Win();		break;
// 	default:
// 		break;
// 	}
// }
// 
// void Game::Render() const {
// 	g_theRHI->GetFontRenderer()->SetFont(L"Fira Code");
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 
// 	switch (m_currentState) {
// 	case GAME_ATTRACT:		Render_Attract();		break;
// 	case CAMERA_SETUP:		Render_Camera_Setup();	break;
// 	case GUARD_SETUP:		Render_Guard_Setup();	break;
// 	case THIEF_SETUP:		Render_Thief_Setup();	break;
// 	case THIEF_MOVE:		Render_Thief_Move();	break;
// 	case GUARD_WAIT:		Render_Guard_Wait();	break;
// 	case GUARD_MOVE:		Render_Guard_Move();	break;
// 	case GUARD_ACT:			Render_Guard_Act();		break;
// 	case THIEF_WIN:			Render_Thief_Win();		break;
// 	case GUARD_WIN:			Render_Guard_Win();		break;
// 	default:
// 		break;
// 	}
// 
// 	if (g_theApp->m_isDebugMode) {
// 		g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 		Vector2 guardCenter((float)m_guardGridCoords.x * GRID_WIDTH_IN_BOARD_WINDOW + 0.5f * GRID_WIDTH_IN_BOARD_WINDOW, (float)m_guardGridCoords.y * GRID_HEIGHT_IN_BOARD_WINDOW + 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW);
// 		Vector2 thiefCenter((float)m_thiefGridCoords.x * GRID_WIDTH_IN_BOARD_WINDOW + 0.5f * GRID_WIDTH_IN_BOARD_WINDOW, (float)m_thiefGridCoords.y * GRID_HEIGHT_IN_BOARD_WINDOW + 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW);
// 
// 		float stretchSizeX = 0.5f * GRID_WIDTH_IN_BOARD_WINDOW * 0.8f;
// 		float stretchSizeY = 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW * 0.8f;
// 
// 		Vector2 guard_BL_Corner = guardCenter + Vector2(-stretchSizeX, -stretchSizeY);
// 		Vector2 guard_BR_Corner = guardCenter + Vector2(stretchSizeX, -stretchSizeY);
// 		Vector2 guard_TL_Corner = guardCenter + Vector2(-stretchSizeX, stretchSizeY);
// 		Vector2 guard_TR_Corner = guardCenter + Vector2(stretchSizeX, stretchSizeY);
// 
// 		Vector2 thief_BL_Corner = thiefCenter + Vector2(-stretchSizeX, -stretchSizeY);
// 		Vector2 thief_BR_Corner = thiefCenter + Vector2(stretchSizeX, -stretchSizeY);
// 		Vector2 thief_TL_Corner = thiefCenter + Vector2(-stretchSizeX, stretchSizeY);
// 		Vector2 thief_TR_Corner = thiefCenter + Vector2(stretchSizeX, stretchSizeY);
// 
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(thief_BL_Corner, guard_BL_Corner, Rgba::GREEN);
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(thief_BR_Corner, guard_BR_Corner, Rgba::GREEN);
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(thief_TL_Corner, guard_TL_Corner, Rgba::GREEN);
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(thief_TR_Corner, guard_TR_Corner, Rgba::GREEN);
// 	}
// 
// 
// 	//--------------------------------------------------------------------
// 	for (auto it = g_tasks.begin(); it != g_tasks.end(); ) {
// 		if (!it->m_stopWatch->HasElapsed()) {
// 			it->Execute();
// 			++it;
// 		}
// 		else {
// 			it = g_tasks.erase(it);
// 		}
// 	}
// }
// 
// Vector2 Game::GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const {
// 	return Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (float)gridCoords.x, GRID_HEIGHT_IN_BOARD_WINDOW * (float)gridCoords.y);
// }
// 
// Vector2 Game::GetGridPositionFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const {
// 	return Vector2(m_gridWidthInThiefWindow * (float)gridCoords.x, m_gridHeightInThiefWindow * (float)gridCoords.y);
// }
// 
// std::string Game::ConvertGameStateToString(eGameState gameState) const {
// 	switch (gameState) {
// 	case GAME_ATTRACT:			return "GAME_ATTRACT";
// 	case CAMERA_SETUP:			return "CAMERA_SETUP";
// 	case GUARD_SETUP:			return "GUARD_SETUP";
// 	case THIEF_SETUP:			return "THIEF_SETUP";
// 	case THIEF_MOVE:			return "THIEF_MOVE";
// 	case GUARD_WAIT:			return "GUARD_WAIT";
// 	case GUARD_MOVE:			return "GUARD_MOVE";
// 	case GUARD_ACT:				return "GUARD_ACT";
// 	case THIEF_WIN:				return "THIEF_WIN";
// 	case GUARD_WIN:				return "GUARD_WIN";
// 	default:					return "UNKOWN";
// 	}
// }
// 
// std::string Game::ConvertActionTypeToString(eActionType actionType) const {
// 	switch (actionType) {
// 	case ACTION_TYPE_FLASHLIGHT:		return "Flashlight";
// 	case ACTION_TYPE_SINGLECAMERA:		return "Check One Camera";
// 	case ACTION_TYPE_SCANCAMERA:		return "Scan All Cameras";
// 	case ACTION_TYPE_MOTIONDETECTOR:	return "Motion Detector";
// 	default:							return "Unknown";
// 	}
// }
// 
// void Game::FindMovableGridsWithinRange(const IntVector2& startCoords, int range, std::vector<IntVector2>& out) {
// 	out.clear();
// 
// 	std::array<int, GRID_COUNT_TOTAL> distances;
// 	std::fill(distances.begin(), distances.end(), 9999);
// 	int startIndex = startCoords.y * GRID_COUNT_X + startCoords.x;
// 	distances[startIndex] = 0;
// 
// 	std::deque<IntVector2> queue;
// 	queue.push_back(startCoords);
// 
// 	while (!queue.empty()) {
// 		IntVector2 thisCoords = queue.front();
// 		queue.pop_front();
// 
// 		int thisIndex = thisCoords.y * GRID_COUNT_X + thisCoords.x;
// 		int thisDistance = distances[thisIndex];
// 
// 		//early out
// 		if (thisDistance >= range) {
// 			continue;
// 		}
// 
// 		IntVector2 eastCoords(thisCoords.x + 1, thisCoords.y);
// 		IntVector2 westCoords(thisCoords.x - 1, thisCoords.y);
// 		IntVector2 northCoords(thisCoords.x, thisCoords.y + 1);
// 		IntVector2 southCoords(thisCoords.x, thisCoords.y - 1);
// 
// 		if (0 <= eastCoords.x && eastCoords.x < GRID_COUNT_X && 0 <= eastCoords.y && eastCoords.y < GRID_COUNT_Y) {
// 			int eastIndex = eastCoords.y * GRID_COUNT_X + eastCoords.x;
// 			eTileType eastType = m_board->m_tiles[eastIndex].m_type;
// 			if (eastType != TILE_TYPE_NONE && eastType != TILE_TYPE_WALL) {
// 				int eastDistance = distances[eastIndex];
// 				if (eastDistance > thisDistance + 1) {
// 					distances[eastIndex] = thisDistance + 1;
// 					queue.push_back(eastCoords);
// 				}
// 			}
// 		}
// 
// 		if (0 <= westCoords.x && westCoords.x < GRID_COUNT_X && 0 <= westCoords.y && westCoords.y < GRID_COUNT_Y) {
// 			int westIndex = westCoords.y * GRID_COUNT_X + westCoords.x;
// 			eTileType westType = m_board->m_tiles[westIndex].m_type;
// 			if (westType != TILE_TYPE_NONE && westType != TILE_TYPE_WALL) {
// 				int westDistance = distances[westIndex];
// 				if (westDistance > thisDistance + 1) {
// 					distances[westIndex] = thisDistance + 1;
// 					queue.push_back(westCoords);
// 				}
// 			}
// 		}
// 
// 		if (0 <= northCoords.x && northCoords.x < GRID_COUNT_X && 0 <= northCoords.y && northCoords.y < GRID_COUNT_Y) {
// 			int northIndex = northCoords.y * GRID_COUNT_X + northCoords.x;
// 			eTileType northType = m_board->m_tiles[northIndex].m_type;
// 			if (northType != TILE_TYPE_NONE && northType != TILE_TYPE_WALL) {
// 				int northDistance = distances[northIndex];
// 				if (northDistance > thisDistance + 1) {
// 					distances[northIndex] = thisDistance + 1;
// 					queue.push_back(northCoords);
// 				}
// 			}
// 		}
// 
// 		if (0 <= southCoords.x && southCoords.x < GRID_COUNT_X && 0 <= southCoords.y && southCoords.y < GRID_COUNT_Y) {
// 			int southIndex = southCoords.y * GRID_COUNT_X + southCoords.x;
// 			eTileType southType = m_board->m_tiles[southIndex].m_type;
// 			if (southType != TILE_TYPE_NONE && southType != TILE_TYPE_WALL) {
// 				int southDistance = distances[southIndex];
// 				if (southDistance > thisDistance + 1) {
// 					distances[southIndex] = thisDistance + 1;
// 					queue.push_back(southCoords);
// 				}
// 			}
// 		}
// 	}
// 
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		if (distances[i] <= range && distances[i] != 0) {
// 			IntVector2 gridCoords(i % GRID_COUNT_X, i / GRID_COUNT_X);
// 			out.push_back(gridCoords);
// 		}
// 	}
// }
// 
// void Game::GenerateDoorsAndWinodwsOpenOrClose() {
// 	m_doorsAndWinodwsStates.clear();
// 
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		eTileType type = m_board->m_tiles[i].m_type;
// 		if (type == TILE_TYPE_DOOR || type == TILE_TYPE_WINDOW) {
// 			int gridX = i % GRID_COUNT_X;
// 			int gridY = i / GRID_COUNT_X;
// 			bool isOpen = CheckRandomChance(0.2f);
// 			m_doorsAndWinodwsStates.insert({ IntVector2(gridX, gridY), isOpen });
// 		}
// 	}
// }
// 
// void Game::CheckIfGuardCanSeeThief() {
// 	Vector2 guardCenter((float)m_guardGridCoords.x * GRID_WIDTH_IN_BOARD_WINDOW + 0.5f * GRID_WIDTH_IN_BOARD_WINDOW, (float)m_guardGridCoords.y * GRID_HEIGHT_IN_BOARD_WINDOW + 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW);
// 	Vector2 thiefCenter((float)m_thiefGridCoords.x * GRID_WIDTH_IN_BOARD_WINDOW + 0.5f * GRID_WIDTH_IN_BOARD_WINDOW, (float)m_thiefGridCoords.y * GRID_HEIGHT_IN_BOARD_WINDOW + 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW);
// 
// 	float stretchSizeX = 0.5f * GRID_WIDTH_IN_BOARD_WINDOW * 0.8f;
// 	float stretchSizeY = 0.5f * GRID_HEIGHT_IN_BOARD_WINDOW * 0.8f;
// 
// 	Vector2 guard_BL_Corner = guardCenter + Vector2(-stretchSizeX, -stretchSizeY);
// 	Vector2 guard_BR_Corner = guardCenter + Vector2(stretchSizeX, -stretchSizeY);
// 	Vector2 guard_TL_Corner = guardCenter + Vector2(-stretchSizeX, stretchSizeY);
// 	Vector2 guard_TR_Corner = guardCenter + Vector2(stretchSizeX, stretchSizeY);
// 
// 	Vector2 thief_BL_Corner = thiefCenter + Vector2(-stretchSizeX, -stretchSizeY);
// 	Vector2 thief_BR_Corner = thiefCenter + Vector2(stretchSizeX, -stretchSizeY);
// 	Vector2 thief_TL_Corner = thiefCenter + Vector2(-stretchSizeX, stretchSizeY);
// 	Vector2 thief_TR_Corner = thiefCenter + Vector2(stretchSizeX, stretchSizeY);
// 
// 	Vector2 direction_BL = (thief_BL_Corner - guard_BL_Corner).GetNormalized();
// 	Vector2 direction_BR = (thief_BR_Corner - guard_BR_Corner).GetNormalized();
// 	Vector2 direction_TL = (thief_TL_Corner - guard_TL_Corner).GetNormalized();
// 	Vector2 direction_TR = (thief_TR_Corner - guard_TR_Corner).GetNormalized();
// 
// 	float distance_BL = GetDistance(thief_BL_Corner, guard_BL_Corner);
// 	float distance_BR = GetDistance(thief_BR_Corner, guard_BR_Corner);
// 	float distance_TL = GetDistance(thief_TL_Corner, guard_TL_Corner);
// 	float distance_TR = GetDistance(thief_TR_Corner, guard_TR_Corner);
// 
// 	RaycastResult2D_t result_BL = m_board->Raycast(guard_BL_Corner, direction_BL, distance_BL);
// 	RaycastResult2D_t result_BR = m_board->Raycast(guard_BR_Corner, direction_BR, distance_BR);
// 	RaycastResult2D_t result_TL = m_board->Raycast(guard_TL_Corner, direction_TL, distance_TL);
// 	RaycastResult2D_t result_TR = m_board->Raycast(guard_TR_Corner, direction_TR, distance_TR);
// 
// 	if (!result_BL.m_didImpact || !result_BR.m_didImpact || !result_TL.m_didImpact || !result_TR.m_didImpact) {
// 		m_isThiefHidden = false;
// 	}
// 	else {
// 		m_isThiefHidden = true;
// 	}
// }
// 
// void Game::CheckIsThiefOnTreasure() {
// 	for (auto it = m_treasures.begin(); it != m_treasures.end(); ) {
// 		if (m_thiefGridCoords == it->first) {
// 			m_isAnyTreasureStolenLastTurn = true;
// 			m_treasureStolenLastTurn = m_thiefGridCoords;
// 			m_treasuresStolenByThief++;
// 			it = m_treasures.erase(it);
// 
// 			Vector2 thiefPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefGridCoords) + 0.5f * Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow);
// 			g_tasks.emplace_back(5.f, m_gameClock.get(), thiefPos, std::bind(&Game::ShowPickUpTreasure, this, std::placeholders::_1, std::placeholders::_2));
// 
// 			break;
// 		}
// 		else {
// 			++it;
// 		}
// 	}
// }
// 
// void Game::CheckIsThiefOnCamera() {
// 	for (auto& camera : m_cameras) {
// 		if (m_thiefGridCoords == camera.first) {
// 			camera.second = false;
// 
// 			Vector2 thiefPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefGridCoords) + 0.5f * Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow);
// 			g_tasks.emplace_back(5.f, m_gameClock.get(), thiefPos, std::bind(&Game::ShowDisconnectCamera, this, std::placeholders::_1, std::placeholders::_2));
// 
// 			break;
// 		}
// 	}
// }
// 
// void Game::CheckIsGuardOnCamera() {
// 	for (auto& camera : m_cameras) {
// 		if (m_guardGridCoords == camera.first) {
// 			camera.second = true;
// 			m_camerasGuardKnows.emplace_back(m_guardGridCoords);
// 
// 			Vector2 cameraPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 			g_tasks.emplace_back(5.f, m_gameClock.get(), cameraPos, std::bind(&Game::ShowCameraIsRecovered, this, std::placeholders::_1, std::placeholders::_2));
// 
// 			break;
// 		}
// 	}
// }
// 
// bool Game::FindPatternInArea(const Pattern_t& pattern, const AABB2& bounds, IntVector2& out) {
// 	BoardData_t* data = m_board->m_boardData.Read();
// 	for (int gridY = bounds.mins.y; gridY <= bounds.maxs.y - (pattern.m_height - 1); ++gridY) {
// 		for (int gridX = bounds.mins.x; gridX <= bounds.maxs.x - (pattern.m_width - 1); ++gridX) {
// 			Rgba color = data->GetColorAtGridCoords(gridX, gridY);
// 			bool flag = true;
// 			for (int y = 0; y < pattern.m_height; ++y) {
// 				for (int x = 0; x < pattern.m_width; ++x) {
// 					Rgba c = data->GetColorAtGridCoords(gridX + x, gridY + y);
// 					int patternIdx = y * pattern.m_width + x;
// 					Rgba patternColor = pattern.m_colors[patternIdx];
// 					if (!c.RgbEqual(patternColor)) {
// 						flag = false;
// 						break;
// 					}
// 				}
// 			}
// 			// find pattern succeed
// 			if (flag) {
// 				out.x = gridX;
// 				out.y = gridY;
// 				return true;
// 			}
// 		}
// 	}
// 	return false;
// }
// 
// void Game::GenerateTreasuresOnBoard() {
// 	m_treasures.clear();
// 	std::vector<IntVector2> floors;
// 	for (int gridY = 0; gridY < GRID_COUNT_Y; ++gridY) {
// 		for (int gridX = 0; gridX < GRID_COUNT_X; ++gridX) {
// 			int gridIndex = gridY * GRID_COUNT_X + gridX;
// 			eTileType type = m_board->m_tiles[gridIndex].m_type;
// 			if (type == TILE_TYPE_FLOOR) {
// 				floors.emplace_back(gridX, gridY);
// 			}
// 		}
// 	}
// 	while (m_treasures.size() < m_totalTreasures) {
// 		int index = GetRandomIntInRange(0, floors.size() - 1);
// 		int gridX = floors[index].x;
// 		int gridY = floors[index].y;
// 
// 		if (m_cameras.find(IntVector2(gridX, gridY)) == m_cameras.end()) {
// 
// 			bool flag = false;
// 			for (auto& t : m_treasures) {
// 				//--------------------------------------------------------------------
// 				// if there is any treasure around the new gridCoords, ignore this gridCoords
// 				if (Abs(t.first.x - gridX) <= 2 && Abs(t.first.y - gridY) <= 2) {
// 					flag = true;
// 					break;
// 				}
// 			}
// 			if (flag) {
// 				continue;
// 			}
// 			else {
// 				m_treasures.insert({ IntVector2(gridX, gridY), false });
// 			}
// 		}
// 	}
// }
// 
// void Game::ClearThiefWindow() const {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
// 	m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
// 	m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_thiefCamera.get());
// 
// 	g_theRHI->GetDevice()->ClearColor(m_thiefCamera->GetRTV(), Rgba::MIDNIGHTBLUE);
// 	g_theRHI->GetDevice()->ClearDepthStencil(m_thiefCamera->GetDSV(), 1.f, 0U);
// }
// 
// void Game::ClearBoardWindow() const {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	g_theRHI->GetDevice()->ClearColor(m_boardCamera->GetRTV(), Rgba::BLACK);
// 	g_theRHI->GetDevice()->ClearDepthStencil(m_boardCamera->GetDSV(), 1.f, 0U);
// }
// 
// void Game::Update_Attract() {
// 	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
// 		m_nextState = CAMERA_SETUP;
// 	}
// }
// 
// void Game::Render_Attract() const {
// 	if (m_board->m_isReady) {
// 		ClearThiefWindow();
// 		g_theRHI->GetFontRenderer()->SetSize(48.f);
// 		g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 
// 		g_theRHI->GetFontRenderer()->DrawTextInBox("Hello, Augmon!\n Press Space to Start...",
// 			AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 		RenderDebugStuffInThiefWindow();
// 
// 
// 		ClearBoardWindow();
// 		g_theRHI->GetFontRenderer()->SetSize(48.f);
// 		g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox("Hello, Augmon!\n Press Space to Start...",
// 			AABB2(Vector2::ZERO, g_boardOutput->GetDimension()));
// 	}
// 	else {
// 		ClearThiefWindow();
// 		g_theRHI->GetFontRenderer()->SetSize(48.f);
// 		g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox("Waiting for initial alignment on board...",
// 			AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 		RenderDebugStuffInThiefWindow();
// 
// 		ClearBoardWindow();
// 		g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 		// Draw alignment quads
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW), Vector2(1.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(0.f, HEIGHT_IN_BOARD_WINDOW), Vector2(0.f, 0.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(WIDTH_IN_BOARD_WINDOW, 0.f), Vector2(1.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 	}
// }
// 
// void Game::Update_Camera_Setup() {
// 	m_cameras.clear();
// 	m_camerasGuardKnows.clear();
// 	BoardData_t* data = m_board->m_boardData.Read();
// 
// 	for (int gridY = 0; gridY < GRID_COUNT_Y; ++gridY) {
// 		for (int gridX = 0; gridX < GRID_COUNT_X; ++gridX) {
// 			int gridIndex = gridY * GRID_COUNT_X + gridX;
// 			if (data->greenFractions[gridIndex] > 0.8f) {
// 				m_cameras.insert({ IntVector2(gridX, gridY), true });
// 				m_camerasGuardKnows.emplace_back(gridX, gridY);
// 			}
// 		}
// 	}
// 	if (m_cameras.size() == m_totalCameras) {
// 		GenerateTreasuresOnBoard();
// 		m_nextState = GUARD_SETUP;
// 	}
// }
// 
// void Game::Render_Camera_Setup() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	RenderAvailableTilesForCameraSetup();
// 	RenderCameraSetupInInfoArea();
// }
// 
// void Game::Update_Guard_Setup() {
// 	Pattern_t guardPattern(1, 1, { Rgba::BLUE });
// 	IntVector2 guardCoords;
// 	bool succeed = FindPatternInArea(guardPattern, AABB2(Vector2(0.f, 4.f), Vector2(GRID_COUNT_X - 1, GRID_COUNT_Y - 1)), guardCoords);
// 	if (succeed) {
// 		m_isGuardOnBoard = true;
// 		m_guardGridCoords = guardCoords;
// 		m_nextState = THIEF_SETUP;
// 	}
// }
// 
// void Game::Render_Guard_Setup() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	ShowAvailableTilesForGuardSetup();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	RenderGuardSetupInInfoArea();
// }
// 
// void Game::Update_Thief_Setup() {
// 	UpdateThiefCursor();
// 
// 	if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_LEFT)) {
// 		int clickGridIndex = m_thiefCursorCoords.y * GRID_COUNT_X + m_thiefCursorCoords.x;
// 		eTileType type = m_board->m_tiles[clickGridIndex].m_type;
// 		if (type == TILE_TYPE_WINDOW || type == TILE_TYPE_DOOR) {
// 			m_isThiefOnBoard = true;
// 			m_isThiefHidden = true;
// 			m_thiefGridCoords = m_thiefCursorCoords;
// 
// 			//--------------------------------------------------------------------
// 			// Guarantee thief's entry point and farthest point are open
// 			m_doorsAndWinodwsStates.at(m_thiefGridCoords) = true;
// 			IntVector2 farthestCoords;
// 			float maxDistanceSquared = 0.f;
// 			for (auto& it : m_doorsAndWinodwsStates) {
// 				float distSquared = GetDistanceSquared(Vector2(it.first), Vector2(m_thiefGridCoords));
// 				if (distSquared > maxDistanceSquared) {
// 					maxDistanceSquared = distSquared;
// 					farthestCoords = it.first;
// 				}
// 			}
// 			m_doorsAndWinodwsStates.at(farthestCoords) = true;
// 
// 			// Thief moves at first
// 			m_nextState = THIEF_MOVE;
// 		}
// 	}
// }
// 
// void Game::Render_Thief_Setup() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefCursor();
// 	ShowAvailableTilesForThiefSetup();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	RenderWhiteTileForGuardOnBoard();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	RenderThiefSetupInInfoArea();
// }
// 
// void Game::Update_Thief_Move() {
// 	static int firstUpdate = true;
// 	if (firstUpdate) {
// 		firstUpdate = false;
// 
// 		// If any treasure stolen by thief
// 		if (m_isAnyTreasureStolenLastTurn) {
// 			m_isAnyTreasureStolenLastTurn = false;
// 
// 			Vector2 treasurePos = GetGridPositionFromGridCoordsOnBoardWindow(m_treasureStolenLastTurn) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 			g_tasks.emplace_back(5.f, m_gameClock.get(), treasurePos, std::bind(&Game::ShowTreasureStolen, this, std::placeholders::_1, std::placeholders::_2));
// 		}
// 	}
// 
// 	UpdateThiefCursor();
// 	FindMovableGridsWithinRange(m_thiefGridCoords, 3, m_thiefAvailableMoveGrids);
// 
// 	if (m_guardUsedFlashlightThisTurn) {
// 		CheckIfGuardCanSeeThief();
// 	}
// 
// 	if (g_theInput->WasMouseJustPressed(InputSystem::MOUSE_LEFT)) {
// 		for (auto& coords : m_thiefAvailableMoveGrids) {
// 			if (m_thiefCursorCoords == coords) {
// 				m_thiefGridCoords = m_thiefCursorCoords;
// 
// 				//--------------------------------------------------------------------
// 				// If thief steps on a treasure tile
// 				CheckIsThiefOnTreasure();
// 
// 				//--------------------------------------------------------------------
// 				// If thief steps on a camera tile, disconnect that camera, and delay guard notification
// 				CheckIsThiefOnCamera();
// 
// 				bool doesThiefWin = CheckThiefWinCondition();
// 				bool doesGuardWin = CheckGuardWinCondition();
// 				if (doesThiefWin) {
// 					m_nextState = THIEF_WIN;
// 				}
// 				else if (doesGuardWin) {
// 					m_nextState = GUARD_WIN;
// 				}
// 				else {
// 					//--------------------------------------------------------------------
// 					// Fixed the flashing bug
// 					m_isThiefHidden = true;
// 
// 					firstUpdate = true;
// 					m_nextState = GUARD_WAIT;
// 				}
// 				break;
// 			}
// 		}
// 	}
// }
// 
// void Game::Render_Thief_Move() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	ShowAvailableTilesForThiefMove();
// 	RenderThiefOnThiefWindow();
// 	RenderThiefCursor();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderThiefMoveInInfoArea();
// 	RenderActionAreaBackground();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	RenderWhiteTileForGuardOnBoard();
// 	if (m_isThiefHidden == false) {
// 		RenderThiefOnBoardWindow();
// 	}
// }
// 
// void Game::Update_Guard_Wait() {
// 	m_guardActionItem = ACTION_TYPE_UNKNOWN;
// 
// 	static bool isFirstUpdate = true;
// 	if (isFirstUpdate) {
// 		// Move to Guard stage
// 
// 		RollDiceForGuards();
// 
// 		// Reset action
// 		m_hasGuardTakenActionThisTurn = false;
// 
// 		// Reset equipment
// 		m_guardUsedFlashlightThisTurn = false;
// 		m_guardEquipment = ACTION_TYPE_UNKNOWN;
// 
// 		isFirstUpdate = false;
// 	}
// 
// 
// 	// If remove guard pawn, goto GUARD_MOVE stage
// 	IntVector2 guardCoords;
// 	bool findGuard = FindPatternInArea(s_guardPattern, AABB2(Vector2(m_guardGridCoords), Vector2(m_guardGridCoords)), guardCoords);
// 	if (!findGuard) {
// 		isFirstUpdate = true;
// 		m_nextState = GUARD_MOVE;
// 		return;
// 	}
// 
// 
// 	// If place down an action, goto GUARD_ACT stage
// 	if (!m_hasGuardTakenActionThisTurn) {
// 		IntVector2 emptyCoords;
// 		bool findActionItem = FindPatternInArea(s_emptyPattern, m_actionItemSlotBounds, emptyCoords);
// 		if (!findActionItem) {
// 			m_nextState = GUARD_ACT;
// 			return;
// 		}
// 	}
// 
// 	IntVector2 unusedCoords;
// 	bool findEquipment = FindPatternInArea(s_flashLightPattern, m_equipmentSlotBounds, unusedCoords);
// 	if (findEquipment) {
// 		m_guardEquipment = ACTION_TYPE_FLASHLIGHT;
// 		if (m_guardUsedFlashlightThisTurn == false) {
// 			if (m_guardFlashlightBatteryRemained > 0) {
// 				m_guardUsedFlashlightThisTurn = true;
// 
// 				Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowEquipFlashlight, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 			else {
// 				Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowFlashlightBatteryIsLow, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 		}
// 	}
// 	if (m_guardUsedFlashlightThisTurn == false) {
// 		m_isThiefHidden = true;
// 	}
// 	else {
// 		CheckIfGuardCanSeeThief();
// 	}
// 	if (m_guardFlashlightBatteryRemained <= 0) {
// 		m_isThiefHidden = true;
// 	}
// }
// 
// void Game::Render_Guard_Wait() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefOnThiefWindow();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	RenderActionAndMovementInInfoArea();
// 	RenderBasicInActionArea();
// 	RenderActionItemNameInActionArea();
// 	RenderEquipmentNameInActionArea();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	if (m_guardEquipment == ACTION_TYPE_FLASHLIGHT) {
// 		RenderFlashlightBatteryInActionArea();
// 	}
// 	RenderWhiteTileForGuardOnBoard();
// 
// 	if (m_isThiefHidden == false) {
// 		RenderThiefOnBoardWindow();
// 	}
// }
// 
// void Game::Update_Guard_Move() {
// 	FindMovableGridsWithinRange(m_guardGridCoords, m_movementOnDiceThisTurn, m_guardAvailableMoveGrids);
// 
// 	m_isGuardOnBoard = false;
// 	Pattern_t guardPattern(1, 1, { Rgba::BLUE });
// 	IntVector2 newGuardCoords;
// 	bool succeed = FindPatternInArea(guardPattern, AABB2(Vector2(0.f, 4.f), Vector2(41.f, 22.f)), newGuardCoords);
// 	if (succeed) {
// 		for (auto& coords : m_guardAvailableMoveGrids) {
// 			if (coords == newGuardCoords) {
// 				m_guardGridCoords = coords;
// 				m_isGuardOnBoard = true;
// 
// 				//--------------------------------------------------------------------
// 				// If guard steps on a camera, recover that camera if its broken
// 				CheckIsGuardOnCamera();
// 
// 				//--------------------------------------------------------------------
// 				bool doesGuardWin = CheckGuardWinCondition();
// 				if (doesGuardWin) {
// 					m_nextState = GUARD_WIN;
// 				}
// 				else {
// 					//--------------------------------------------------------------------
// 					// check guards equipment, decrease battery life
// 					if (m_guardUsedFlashlightThisTurn && m_guardFlashlightBatteryRemained > 0) {
// 						m_guardFlashlightBatteryRemained--;
// 					}
// 					m_nextState = THIEF_MOVE;
// 				}
// 				break;
// 			}
// 		}
// 	}
// }
// 
// void Game::Render_Guard_Move() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefOnThiefWindow();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	RenderActionAndMovementInInfoArea();
// 	RenderBasicInActionArea();
// 	RenderActionItemNameInActionArea();
// 	RenderEquipmentNameInActionArea();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	if (m_guardEquipment == ACTION_TYPE_FLASHLIGHT) {
// 		RenderFlashlightBatteryInActionArea();
// 	}
// 	if (m_isThiefHidden == false) {
// 		RenderThiefOnBoardWindow();
// 	}
// 
// 	if (m_isGuardOnBoard) {
// 		RenderWhiteTileForGuardOnBoard();
// 	}
// 	else {
// 		ShowAvailableTilesForGuardMove();
// 	}
// }
// 
// void Game::Update_Guard_Act() {
// 	if (m_guardActionItem == ACTION_TYPE_UNKNOWN || m_camerasGuardKnows.empty()) {
// 		IntVector2 coords;
// 		// Nothing on action item slot
// 		if (FindPatternInArea(s_emptyPattern, m_actionItemSlotBounds, coords)) {
// 			m_nextState = GUARD_WAIT;
// 		}
// 
// 		if (!m_hasGuardTakenActionThisTurn) {
// 			if (FindPatternInArea(s_singleCameraPattern, m_actionItemSlotBounds, coords)) {
// 				for (auto it = m_guardActionsInHand.begin(); it != m_guardActionsInHand.end(); ) {
// 					if (*it == ACTION_TYPE_SINGLECAMERA) {
// 						m_guardActionItem = ACTION_TYPE_SINGLECAMERA;
// 						// Remove only one action in hand
// 						it = m_guardActionsInHand.erase(std::remove(m_guardActionsInHand.begin(), m_guardActionsInHand.end(), ACTION_TYPE_SINGLECAMERA));
// 
// 						if (m_camerasGuardKnows.empty()) {
// 							Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 							g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowNoCameraAvailable, this, std::placeholders::_1, std::placeholders::_2));
// 						}
// 
// 						Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 						g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowCheckOneCamera, this, std::placeholders::_1, std::placeholders::_2));
// 
// 						break;
// 					}
// 					else {
// 						++it;
// 					}
// 				}
// 			}
// 			else if (FindPatternInArea(s_scanAllCameraPattern, m_actionItemSlotBounds, coords)) {
// 				for (auto it = m_guardActionsInHand.begin(); it != m_guardActionsInHand.end(); ) {
// 					if (*it == ACTION_TYPE_SCANCAMERA) {
// 						m_guardActionItem = ACTION_TYPE_SCANCAMERA;
// 						// Remove only one action in hand
// 						it = m_guardActionsInHand.erase(std::remove(m_guardActionsInHand.begin(), m_guardActionsInHand.end(), ACTION_TYPE_SCANCAMERA));
// 
// 						if (m_camerasGuardKnows.empty()) {
// 							Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 							g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowNoCameraAvailable, this, std::placeholders::_1, std::placeholders::_2));
// 						}
// 
// 						Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 						g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowScanAllCameras, this, std::placeholders::_1, std::placeholders::_2));
// 
// 						break;
// 					}
// 					else {
// 						++it;
// 					}
// 				}
// 			}
// 			else if (FindPatternInArea(s_motionDetectorPattern, m_actionItemSlotBounds, coords)) {
// 				for (auto it = m_guardActionsInHand.begin(); it != m_guardActionsInHand.end(); ) {
// 					if (*it == ACTION_TYPE_MOTIONDETECTOR) {
// 						m_guardActionItem = ACTION_TYPE_MOTIONDETECTOR;
// 						// Remove only one action in hand
// 						it = m_guardActionsInHand.erase(std::remove(m_guardActionsInHand.begin(), m_guardActionsInHand.end(), ACTION_TYPE_MOTIONDETECTOR));
// 
// 						Vector2 guardPos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 						g_tasks.emplace_back(5.f, m_gameClock.get(), guardPos, std::bind(&Game::ShowMotionDetector, this, std::placeholders::_1, std::placeholders::_2));
// 
// 						break;
// 					}
// 					else {
// 						++it;
// 					}
// 				}
// 			}
// 		}
// 	}
// 	else if (m_guardActionItem == ACTION_TYPE_SINGLECAMERA) {
// 		IntVector2 coords;
// 		if (FindPatternInArea(s_cameraPattern, AABB2(Vector2(0.f, 4.f), Vector2(GRID_COUNT_X - 1, GRID_COUNT_Y - 1)), coords)) {
// 			if (m_cameras.at(coords) == false) {
// 				// camera is off, notifies the guard
// 				m_camerasGuardKnows.erase(std::remove(m_camerasGuardKnows.begin(), m_camerasGuardKnows.end(), coords), m_camerasGuardKnows.end());
// 
// 				Vector2 cameraPos = GetGridPositionFromGridCoordsOnBoardWindow(coords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), cameraPos, std::bind(&Game::ShowCameraIsDisconnected, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 			else {
// 				// camera is on, check if camera can see thief
// 				bool canSeeThief = true;
// 				if (coords.x == m_thiefGridCoords.x || coords.y == m_thiefGridCoords.y) {
// 					Vector2 cameraCenter = GetGridPositionFromGridCoordsOnBoardWindow(coords);
// 					Vector2 thiefCenter = GetGridPositionFromGridCoordsOnBoardWindow(m_thiefGridCoords);
// 					RaycastResult2D_t result = m_board->Raycast(cameraCenter, (thiefCenter - cameraCenter).GetNormalized(), (thiefCenter - cameraCenter).GetLength());
// 					if (!result.m_didImpact) {
// 						m_isThiefHidden = false;
// 					}
// 					else {
// 						canSeeThief = false;
// 					}
// 				}
// 				else {
// 					canSeeThief = false;
// 				}
// 				if (!canSeeThief) {
// 					Vector2 cameraPos = GetGridPositionFromGridCoordsOnBoardWindow(coords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 					g_tasks.emplace_back(5.f, m_gameClock.get(), cameraPos, std::bind(&Game::ShowCameraCannotSeeThief, this, std::placeholders::_1, std::placeholders::_2));
// 				}
// 			}
// 			m_hasGuardTakenActionThisTurn = true;
// 			m_guardActionItem = ACTION_TYPE_UNKNOWN;
// 		}
// 	}
// 	else if (m_guardActionItem == ACTION_TYPE_SCANCAMERA) {
// 		for (auto it = m_camerasGuardKnows.begin(); it != m_camerasGuardKnows.end(); ) {
// 			IntVector2 coords = *it;
// 			if (m_cameras.at(coords) == false) {
// 				// camera is off, notifies the guard
// 				it = m_camerasGuardKnows.erase(it);
// 
// 				Vector2 cameraPos = GetGridPositionFromGridCoordsOnBoardWindow(coords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), cameraPos, std::bind(&Game::ShowCameraIsDisconnected, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 			else {
// 				// camera is on, check if camera can see thief
// 				bool canSeeThief = true;
// 				if (coords.x == m_thiefGridCoords.x || coords.y == m_thiefGridCoords.y) {
// 					Vector2 cameraCenter = GetGridPositionFromGridCoordsOnBoardWindow(coords);
// 					Vector2 thiefCenter = GetGridPositionFromGridCoordsOnBoardWindow(m_thiefGridCoords);
// 					RaycastResult2D_t result = m_board->Raycast(cameraCenter, (thiefCenter - cameraCenter).GetNormalized(), (thiefCenter - cameraCenter).GetLength());
// 					if (!result.m_didImpact) {
// 						m_isThiefHidden = false;
// 					}
// 					else {
// 						canSeeThief = false;
// 					}
// 				}
// 				else {
// 					canSeeThief = false;
// 				}
// 				if (!canSeeThief) {
// 					Vector2 cameraPos = GetGridPositionFromGridCoordsOnBoardWindow(coords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
// 					g_tasks.emplace_back(5.f, m_gameClock.get(), cameraPos, std::bind(&Game::ShowCameraCannotSeeThief, this, std::placeholders::_1, std::placeholders::_2));
// 				}
// 				++it;
// 			}
// 		}
// 		m_hasGuardTakenActionThisTurn = true;
// 		m_guardActionItem = ACTION_TYPE_UNKNOWN;
// 	}
// 	else if (m_guardActionItem == ACTION_TYPE_MOTIONDETECTOR) {
// 		static float timer = 0.f;
// 		timer += m_gameClock->GetDeltaSeconds();
// 		m_isThiefInRoom = false;
// 		for (int i = 0; i < m_rooms.size(); ++i) {
// 			if (m_rooms[i].IsPointInside(Vector2(m_thiefGridCoords))) {
// 				m_isThiefInRoom = true;
// 				m_theRoomThiefIsIn = m_rooms[i];
// 				break;
// 			}
// 		}
// 		if (timer > 5.0f) {
// 			timer = 0.f;
// 			m_hasGuardTakenActionThisTurn = true;
// 			m_guardActionItem = ACTION_TYPE_UNKNOWN;
// 		}
// 	}
// }
// 
// void Game::Render_Guard_Act() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderCameraOnThief();
// 	RenderTreasuresOnThief();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefOnThiefWindow();
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	RenderMapOnBoardWindow();
// 	RenderInfoAreaBackground();
// 	RenderActionAreaBackground();
// 	RenderActionAndMovementInInfoArea();
// 	RenderBasicInActionArea();
// 	RenderActionItemNameInActionArea();
// 	RenderEquipmentNameInActionArea();
// 	RenderCamerasOnBoard();
// 	RenderTreasuresOnBoard();
// 	if (m_guardEquipment == ACTION_TYPE_FLASHLIGHT) {
// 		RenderFlashlightBatteryInActionArea();
// 	}
// 	RenderWhiteTileForGuardOnBoard();
// 
// 	if (m_isThiefHidden == false) {
// 		RenderThiefOnBoardWindow();
// 	}
// 
// 	if (m_guardActionItem == ACTION_TYPE_SINGLECAMERA) {
// 		ShowAvailableTilesForCameras();
// 	}
// 	else if (m_guardActionItem == ACTION_TYPE_MOTIONDETECTOR) {
// 		ShowWhiteTilesForTheRoomThiefIsIn();
// 	}
// }
// 
// void Game::Update_Thief_Win() {
// 	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
// 		m_nextState = GAME_ATTRACT;
// 		m_guardActionsInHand.clear();
// 	}
// }
// 
// void Game::Render_Thief_Win() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefOnThiefWindow();
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(g_thiefOutput->GetWidth(), g_thiefOutput->GetHeight()), Rgba(0.6f, 0.f, 0.f, .5f));
// 	g_theRHI->GetFontRenderer()->SetSize(64.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief Wins!", AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(g_boardOutput->GetWidth(), g_boardOutput->GetHeight()), Rgba(0.6f, 0.f, 0.f, 1.f));
// 	g_theRHI->GetFontRenderer()->SetSize(64.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief Wins!", AABB2(Vector2::ZERO, g_boardOutput->GetDimension()));
// }
// 
// void Game::Update_Guard_Win() {
// 	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
// 		m_nextState = GAME_ATTRACT;
// 		m_guardActionsInHand.clear();
// 	}
// }
// 
// void Game::Render_Guard_Win() const {
// 	//-------------------------------Thief------------------------------
// 	ClearThiefWindow();
// 	RenderMapOnThiefWindow();
// 	RenderGuardsOnThiefWindow();
// 	RenderDebugStuffInThiefWindow();
// 	RenderThiefOnThiefWindow();
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(g_thiefOutput->GetWidth(), g_thiefOutput->GetHeight()), Rgba(0.f, 0.f, 0.6f, .5f));
// 	g_theRHI->GetFontRenderer()->SetSize(64.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Guard Wins!", AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 
// 	//-------------------------------Guard------------------------------
// 	ClearBoardWindow();
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2::ZERO, Vector2(0.f, 1.f), Vector2(g_boardOutput->GetWidth(), g_boardOutput->GetHeight()), Rgba(0.f, 0.f, 0.6f, 1.f));
// 	g_theRHI->GetFontRenderer()->SetSize(64.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Guard Wins!", AABB2(Vector2::ZERO, g_boardOutput->GetDimension()));
// }
// 
// void Game::RenderDebugStuffInThiefWindow() const {
// 	std::string debugStr = "CurrentState: " + ConvertGameStateToString(m_currentState) + "\n" + Stringf("Treasures stolen: %d", m_treasuresStolenByThief);
// 	g_theRHI->GetFontRenderer()->SetSize(16.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_TOP);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(debugStr.c_str(), AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 
// 	std::string mouseStr = Stringf("Mouse Coords: [%d, %d]", m_thiefCursorCoords.x, m_thiefCursorCoords.y);
// 	//Vector2 mouseClientPos = g_theInput->GetMouseClientPos(g_thiefOutput->m_window->GetHandle());
// 	//std::string mouseStr = Stringf("Mouse Coords: [%d, %d]", (int)mouseClientPos.x, (int)mouseClientPos.y);
// 
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_RIGHT, TEXT_ALIGN_TOP);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(mouseStr.c_str(), AABB2(Vector2::ZERO, g_thiefOutput->GetDimension()));
// 
// }
// 
// void Game::RenderGridLines() const {
// 	for (int i = 0; i < GRID_COUNT_X; ++i) {
// 		Vector2 startPos((float)i * GRID_WIDTH_IN_BOARD_WINDOW, 0.f);
// 		Vector2 endPos((float)i * GRID_WIDTH_IN_BOARD_WINDOW, HEIGHT_IN_BOARD_WINDOW);
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos, endPos, Rgba::BLACK);
// 	}
// 
// 	for (int i = 0; i < GRID_COUNT_Y; ++i) {
// 		Vector2 startPos(0.f, (float)i * GRID_HEIGHT_IN_BOARD_WINDOW);
// 		Vector2 endPos(WIDTH_IN_BOARD_WINDOW, (float)i * GRID_HEIGHT_IN_BOARD_WINDOW);
// 		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos, endPos, Rgba::BLACK);
// 	}
// }
// 
// void Game::RenderInfoAreaBackground() const {
// 	g_theRHI->GetFontRenderer()->SetSize(32.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.GetDimensions().x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.GetDimensions().y), Rgba(1.f, 1.f, 0.f, 0.3f));
// 	// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Hello, Augmon!\n This is the Info Area!",
// 	// 		AABB2(Vector2(GRID_WIDTH_IN_WINDOW* 30.f, GRID_WIDTH_IN_WINDOW * 14.f), Vector2(GRID_WIDTH_IN_WINDOW * 40.f, GRID_HEIGHT_IN_WINDOW* 19.f)));
// }
// 
// void Game::RenderActionAreaBackground() const {
// 	g_theRHI->GetFontRenderer()->SetSize(32.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_actionAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_actionAreaBounds.mins.y), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_actionAreaBounds.GetDimensions().x, GRID_HEIGHT_IN_BOARD_WINDOW * m_actionAreaBounds.GetDimensions().y), Rgba(0.f, 1.f, 1.f, 0.3f));
// 	// 	g_theRHI->GetFontRenderer()->DrawTextInBox("This is the Action Area!\n Used to put LEGO action items!",
// 	// 		AABB2(Vector2(GRID_WIDTH_IN_WINDOW* 30.f, GRID_WIDTH_IN_WINDOW * 7.f), Vector2(GRID_WIDTH_IN_WINDOW * 40.f, GRID_HEIGHT_IN_WINDOW* 14.f)));
// }
// 
// void Game::RenderMapOnBoardWindow() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(m_board->m_tileMesh.get());
// }
// 
// void Game::RenderMapOnThiefWindow() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(m_tileMesh.get());
// }
// 
// void Game::ShowAvailableTilesForGuardSetup() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > 1.f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		if (m_board->m_tiles[i].m_type == TILE_TYPE_FLOOR) {
// 			int gridX = i % GRID_COUNT_X;
// 			int gridY = i / GRID_COUNT_X;
// 			if (m_cameras.find(IntVector2(gridX, gridY)) != m_cameras.end()) {
// 				continue;
// 			}
// 			Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(gridX, gridY));
// 			g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba(1.f, 1.f, 1.f, fraction));
// 		}
// 	}
// }
// 
// void Game::RebuildTileMeshOnThief() {
// 	if (!m_tileMesh) {
// 		m_tileMesh = std::make_unique<Mesh>(PRIMITIVE_TYPE_TRIANGLELIST, true);
// 	}
// 	else {
// 		m_tileMesh->Reset(PRIMITIVE_TYPE_TRIANGLELIST, true);
// 	}
// 
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		int index = m_board->m_tiles[i].m_indexInTileset;
// 		// empty
// 		if (index == 0) {
// 			continue;
// 		}
// 		AABB2 uv = m_board->m_tileSet->GetUVFromIndex(index);
// 		int gridY = i / GRID_COUNT_X;
// 		int gridX = i % GRID_COUNT_X;
// 		Vector2 position(m_gridWidthInThiefWindow * gridX, m_gridHeightInThiefWindow * gridY);
// 
// 		m_tileMesh->AddQuad(position, Vector2(0.f, 1.f), m_gridWidthInThiefWindow, m_gridHeightInThiefWindow, uv);
// 	}
// }
// 
// void Game::RenderGuardsOnThiefWindow() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	Vector2 guardPosInThiefWindow = GetGridPositionFromGridCoordsOnThiefWindow(m_guardGridCoords);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(guardPosInThiefWindow, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), Rgba::BLUE);
// }
// 
// void Game::RenderWhiteTileForGuardOnBoard() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	Vector2 guardPosition = GetGridPositionFromGridCoordsOnBoardWindow(m_guardGridCoords);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(guardPosition, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::WHITE);
// }
// 
// void Game::UpdateThiefCursor() {
// 	Vector2 mouseClientPos = g_theInput->GetMouseClientPos(g_thiefOutput->m_window->GetHandle());
// 	mouseClientPos.x = ClampFloat(mouseClientPos.x, 0.f, g_thiefOutput->GetWidth() - 1);
// 	mouseClientPos.y = ClampFloat(mouseClientPos.y, 0.f, g_thiefOutput->GetHeight() - 1);
// 
// 	m_thiefCursorCoords.x = (int)(mouseClientPos.x / m_gridWidthInThiefWindow);
// 	float height = g_thiefOutput->GetHeight();
// 	m_thiefCursorCoords.y = (int)((height - mouseClientPos.y) / m_gridHeightInThiefWindow);
// }
// 
// void Game::RenderThiefCursor() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	Vector2 cursorPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefCursorCoords);
// 	g_theRHI->GetImmediateRenderer()->DrawLineBox2D(AABB2(cursorPos, cursorPos + Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow)), Rgba::RED);
// }
// 
// void Game::RenderThiefMoveInInfoArea() const {
// 	AABB2 areaBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(48.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Thief Move Stage", areaBounds);
// }
// 
// void Game::RenderThiefSetupInInfoArea() const {
// 	AABB2 areaBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(48.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Wait for Thief setup", areaBounds);
// }
// 
// void Game::RenderGuardSetupInInfoArea() const {
// 	AABB2 areaBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(48.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Wait for Guard setup", areaBounds);
// }
// 
// void Game::ShowAvailableTilesForThiefSetup() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > 1.f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		eTileType type = m_board->m_tiles[i].m_type;
// 		if (type == TILE_TYPE_DOOR || type == TILE_TYPE_WINDOW) {
// 			int gridX = i % GRID_COUNT_X;
// 			int gridY = i / GRID_COUNT_X;
// 			Vector2 pos = GetGridPositionFromGridCoordsOnThiefWindow(IntVector2(gridX, gridY));
// 			g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), Rgba(1.f, 1.f, 1.f, fraction));
// 		}
// 	}
// }
// 
// void Game::ShowAvailableTilesForThiefMove() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > 1.f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (int i = 0; i < m_thiefAvailableMoveGrids.size(); ++i) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefAvailableMoveGrids[i]);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), Rgba(1.f, 1.f, 1.f, fraction));
// 	}
// }
// 
// void Game::RenderThiefOnThiefWindow() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	Vector2 thiefPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefGridCoords);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(thiefPos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), Rgba::RED);
// }
// 
// void Game::RenderThiefOnBoardWindow() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	Vector2 thiefPos = GetGridPositionFromGridCoordsOnBoardWindow(m_thiefGridCoords);
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(thiefPos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// }
// 
// void Game::ShowAvailableTilesForGuardMove() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > 1.f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (int i = 0; i < m_guardAvailableMoveGrids.size(); ++i) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(m_guardAvailableMoveGrids[i]);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba(1.f, 1.f, 1.f, fraction));
// 	}
// }
// 
// bool Game::CheckThiefWinCondition() {
// 	if (m_doorsAndWinodwsStates.find(m_thiefGridCoords) != m_doorsAndWinodwsStates.end()) {
// 		bool isOpen = m_doorsAndWinodwsStates.at(m_thiefGridCoords);
// 		if (isOpen && m_treasuresStolenByThief >= m_leastTreasuresStolenToWin) {
// 			return true;
// 		}
// 		else {
// 			if (m_treasuresStolenByThief < m_leastTreasuresStolenToWin) {
// 				Vector2 thiefPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefGridCoords) + 0.5f * Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), thiefPos, std::bind(&Game::ShowNeedMoreTreasureText, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 			else {
// 				Vector2 thiefPos = GetGridPositionFromGridCoordsOnThiefWindow(m_thiefGridCoords) + 0.5f * Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow);
// 				g_tasks.emplace_back(5.f, m_gameClock.get(), thiefPos, std::bind(&Game::ShowLockedText, this, std::placeholders::_1, std::placeholders::_2));
// 			}
// 			return false;
// 		}
// 	}
// 	return false;
// }
// 
// bool Game::CheckGuardWinCondition() {
// 	if (m_guardGridCoords == m_thiefGridCoords) {
// 		return true;
// 	}
// 	return false;
// }
// 
// void Game::RollDiceForGuards() {
// 	m_movementOnDiceThisTurn = GetRandomIntInRange(1, 6);
// 	m_actionOnDiceThisTurn = (eActionType)GetRandomIntInRange(2, 3); // 0 is unknown, 1 is equipment
// 	m_guardActionsInHand.emplace_back(m_actionOnDiceThisTurn);
// }
// 
// void Game::RenderActionAndMovementInInfoArea() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	AABB2 areaBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 	AABB2 actionNameBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.maxs.y - 1)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 	AABB2 movementDescBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.maxs.y - 1)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.maxs.y)));
// 	AABB2 movementBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.maxs.y)));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	std::string actionName;
// 
// 	switch (m_actionOnDiceThisTurn) {
// 	case ACTION_TYPE_FLASHLIGHT:
// 		actionName = ConvertActionTypeToString(ACTION_TYPE_FLASHLIGHT);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox(actionName, actionNameBounds);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::BLUE);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::BLUE);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::YELLOW);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::YELLOW);
// 		break;
// 	case ACTION_TYPE_SINGLECAMERA:
// 		actionName = ConvertActionTypeToString(ACTION_TYPE_SINGLECAMERA);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox(actionName, actionNameBounds);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::BLUE);
// 		break;
// 	case ACTION_TYPE_SCANCAMERA:
// 		actionName = ConvertActionTypeToString(ACTION_TYPE_SCANCAMERA);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox(actionName, actionNameBounds);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::RED);
// 		break;
// 	case ACTION_TYPE_MOTIONDETECTOR:
// 		actionName = ConvertActionTypeToString(ACTION_TYPE_MOTIONDETECTOR);
// 		g_theRHI->GetFontRenderer()->DrawTextInBox(actionName, actionNameBounds);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::YELLOW);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 1.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::YELLOW);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 4.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::GREEN);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.x + 5.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_infoAreaBounds.mins.y + 2.f)), Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), Rgba::GREEN);
// 		break;
// 	default:
// 		g_theRHI->GetFontRenderer()->DrawTextInBox("UNKOWN", actionNameBounds);
// 		break;
// 	}
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Max move distance this turn:", movementDescBounds);
// 	g_theRHI->GetFontRenderer()->SetSize(64.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(Stringf("%d", m_movementOnDiceThisTurn), movementBounds);
// }
// 
// void Game::RenderBasicInActionArea() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 
// 	//-----------------------------------------------------------------------------------------------------------
// 	// Background
// 
// 	Rgba backgroundColor = Rgba::WHITE;
// 	// action slot
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_actionItemSlotBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_actionItemSlotBounds.mins.y), Vector2(0.f, 1.f), 2.f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), backgroundColor);
// 
// 	// equipment slot
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_equipmentSlotBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_equipmentSlotBounds.mins.y), Vector2(0.f, 1.f), 2.f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), backgroundColor);
// 
// 	//-----------------------------------------------------------------------------------------------------------
// 	// Text
// 	AABB2 actionDescBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionItemSlotBounds.mins.x), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.maxs.y - 1)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * m_actionAreaBounds.maxs.y));
// 	AABB2 equipmentDescBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.maxs.y - 1)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.maxs.x), GRID_HEIGHT_IN_BOARD_WINDOW * m_actionAreaBounds.maxs.y));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Action Item", actionDescBounds);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Equipment", equipmentDescBounds);
// }
// 
// void Game::RenderActionItemNameInActionArea() const {
// 	AABB2 actionDescBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionItemSlotBounds.mins.x), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.y)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.y + 1.f)));
// 
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
// 	std::string actionItemName = ConvertActionTypeToString(m_guardActionItem);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(actionItemName.c_str(), actionDescBounds);
// 
// }
// 
// void Game::RenderEquipmentNameInActionArea() const {
// 	AABB2 equipmentDescBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.x + 11.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.y)), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (m_actionAreaBounds.maxs.x), GRID_HEIGHT_IN_BOARD_WINDOW * (m_actionAreaBounds.mins.y + 1.f)));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
// 	std::string equipmentName = ConvertActionTypeToString(m_guardEquipment);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(equipmentName.c_str(), equipmentDescBounds);
// }
// 
// void Game::RenderFlashlightBatteryInActionArea() const {
// 	float remainedBatteryFraction = (float)m_guardFlashlightBatteryRemained / (float)m_guardFlashlightBatteryTotal;
// 
// 	Vector2 batteryBoundsStart(GRID_WIDTH_IN_BOARD_WINDOW * (m_equipmentSlotBounds.maxs.x + 2.f), GRID_HEIGHT_IN_BOARD_WINDOW * (m_equipmentSlotBounds.mins.y));
// 	Vector2 batteryBoundsEnd = batteryBoundsStart + Vector2(GRID_WIDTH_IN_BOARD_WINDOW * 1.f, GRID_HEIGHT_IN_BOARD_WINDOW * 2.f);
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 
// 	Rgba startColor = Rgba::RED;
// 	Rgba endColor = Rgba::GREEN;
// 	Rgba batteryColor = Interpolate(startColor, endColor, remainedBatteryFraction);
// 
// 	g_theRHI->GetImmediateRenderer()->DrawLineBox2D(AABB2(batteryBoundsStart, batteryBoundsEnd));
// 	g_theRHI->GetImmediateRenderer()->DrawQuad2D(batteryBoundsStart, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, 2.f * remainedBatteryFraction * GRID_HEIGHT_IN_BOARD_WINDOW), batteryColor);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(30.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(Stringf("%d", m_guardFlashlightBatteryRemained), AABB2(batteryBoundsStart, batteryBoundsEnd));
// }
// 
// void Game::RenderAvailableTilesForCameraSetup() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
// 		if (m_board->m_tiles[i].m_type == TILE_TYPE_FLOOR) {
// 			int gridX = i % GRID_COUNT_X;
// 			int gridY = i / GRID_COUNT_X;
// 			Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(gridX, gridY));
// 			g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW));
// 		}
// 	}
// }
// 
// void Game::RenderWhiteTileForCamerasOnBoard() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 	for (auto& cam : m_cameras) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(cam.first);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW));
// 	}
// }
// 
// void Game::RenderCamerasOnBoard() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_cameraSpriteCoords);
// 	for (auto& cam : m_cameras) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(cam.first);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), uv);
// 		if (std::find(m_camerasGuardKnows.begin(), m_camerasGuardKnows.end(), cam.first) == m_camerasGuardKnows.end()) {
// 			AABB2 unkownUV = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_unknownSpriteCoords);
// 			g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), unkownUV);
// 		}
// 	}
// }
// 
// void Game::RenderCameraOnThief() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_cameraSpriteCoords);
// 	for (auto& cam : m_cameras) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnThiefWindow(cam.first);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), uv);
// 		if (cam.second == false) {
// 			AABB2 unkownUV = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_unknownSpriteCoords);
// 			g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), unkownUV);
// 		}
// 	}
// }
// 
// void Game::RenderCameraSetupInInfoArea() const {
// 	AABB2 areaBounds(Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.mins.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.mins.y), Vector2(GRID_WIDTH_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.x, GRID_HEIGHT_IN_BOARD_WINDOW * m_infoAreaBounds.maxs.y));
// 
// 	g_theRHI->GetFontRenderer()->SetSize(48.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox(Stringf("Wait for cameras setup: %d left", m_totalCameras - m_cameras.size()), areaBounds);
// }
// 
// void Game::ShowAvailableTilesForCameras() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > 1.f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 
// 	if (!m_camerasGuardKnows.empty()) {
// 		Mesh mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
// 		for (auto cam : m_camerasGuardKnows) {
// 			Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(cam);
// 			mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 1.f, fraction));
// 		}
// 		g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
// 	}
// }
// 
// void Game::ShowWhiteTilesForTheRoomThiefIsIn() const {
// 	static bool direction = true;
// 	static float fraction = 0.f;
// 	if (fraction > .5f) {
// 		direction = false;
// 	}
// 	if (fraction < 0.f) {
// 		direction = true;
// 	}
// 	if (direction == true) {
// 		fraction += m_gameClock->GetDeltaSeconds();
// 	}
// 	else {
// 		fraction -= m_gameClock->GetDeltaSeconds();
// 	}
// 
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
// 
// 	if (m_isThiefInRoom) {
// 		Mesh mesh(PRIMITIVE_TYPE_TRIANGLELIST, true);
// 
// 		for (int x = m_theRoomThiefIsIn.mins.x; x <= m_theRoomThiefIsIn.maxs.x; ++x) {
// 			for (int y = m_theRoomThiefIsIn.mins.y; y <= m_theRoomThiefIsIn.maxs.y; ++y) {
// 				Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(IntVector2(x, y));
// 				mesh.AddQuad(pos, Vector2(0.f, 1.f), GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW, Rgba(1.f, 1.f, 1.f, fraction));
// 
// 			}
// 		}
// 		g_theRHI->GetImmediateRenderer()->DrawMeshImmediate(&mesh);
// 	}
// }
// 
// void Game::RenderTreasuresOnBoard() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_treasureSpriteCoords);
// 	for (auto& t : m_treasures) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(t.first);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), uv);
// 	}
// 	if (m_isAnyTreasureStolenLastTurn) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnBoardWindow(m_treasureStolenLastTurn);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW), uv);
// 	}
// }
// 
// void Game::RenderTreasuresOnThief() const {
// 	g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("terrain"));
// 	AABB2 uv = g_theResourceManager->GetSpriteSheet("terrain_32x32")->GetUVFromSpriteCoords(s_treasureSpriteCoords);
// 	for (auto& t : m_treasures) {
// 		Vector2 pos = GetGridPositionFromGridCoordsOnThiefWindow(t.first);
// 		g_theRHI->GetImmediateRenderer()->DrawQuad2D(pos, Vector2(0.f, 1.f), Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow), uv);
// 	}
// }
// 
// void Game::ShowNeedMoreTreasureText(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
// 	m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
// 	m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_thiefCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Need steal more treasure!", bounds);
// }
// 
// void Game::ShowLockedText(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
// 	m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
// 	m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_thiefCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Locked!", bounds);
// }
// 
// void Game::ShowEquipFlashlight(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Equip Flashlight", bounds);
// }
// 
// void Game::ShowFlashlightBatteryIsLow(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Flashlight is low!", bounds, Rgba::RED);
// }
// 
// void Game::ShowCheckOneCamera(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Check one camera", bounds);
// }
// 
// void Game::ShowScanAllCameras(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Scan all cameras", bounds);
// }
// 
// void Game::ShowCameraCannotSeeThief(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Clear!", bounds, Rgba::GREEN);
// }
// 
// void Game::ShowCameraIsDisconnected(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Disconnected!", bounds, Rgba::RED);
// }
// 
// void Game::ShowCameraIsRecovered(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Recovered", bounds, Rgba::GREEN);
// }
// 
// void Game::ShowNoCameraAvailable(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("No camera responds!", bounds, Rgba::YELLOW);
// }
// 
// void Game::ShowPickUpTreasure(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
// 	m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
// 	m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_thiefCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("You picked up a treasure!", bounds, Rgba::YELLOW);
// }
// 
// void Game::ShowDisconnectCamera(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_thiefOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_thiefOutput.get());
// 	m_thiefCamera->SetRenderTarget(g_thiefOutput->GetRTV());
// 	m_thiefCamera->SetDepthTarget(g_thiefOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_thiefCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("You disconnected a camera!", bounds, Rgba::RED);
// }
// 
// void Game::ShowTreasureStolen(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Treasure was stolen!", bounds, Rgba::RED);
// }
// 
// void Game::ShowMotionDetector(float normalizedTime, const Vector2& pos) {
// 	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
// 	g_theRHI->GetImmediateRenderer()->BindOutput(g_boardOutput.get());
// 	m_boardCamera->SetRenderTarget(g_boardOutput->GetRTV());
// 	m_boardCamera->SetDepthTarget(g_boardOutput->GetDSV());
// 	g_theRHI->GetImmediateRenderer()->BindCamera(m_boardCamera.get());
// 
// 	AABB2 bounds(pos + normalizedTime * Vector2(0.f, 50.f), 200.f, 50.f);
// 
// 	g_theRHI->GetFontRenderer()->SetSize(24.f);
// 	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
// 	g_theRHI->GetFontRenderer()->DrawTextInBox("Motion Detector!", bounds, Rgba::WHITE);
// }
