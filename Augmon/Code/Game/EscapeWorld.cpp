#include "Game/EscapeWorld.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "ThirdParty/pugixml/pugixml.hpp"


const Pattern_t EscapeWorld::s_monitorPattern(1, 1, { Rgba::GREEN });
const Pattern_t EscapeWorld::s_emptyPattern(2, 2, { Rgba::WHITE, Rgba::WHITE, Rgba::WHITE, Rgba::WHITE });
const Pattern_t EscapeWorld::s_flashLightPattern(2, 2, { Rgba::BLUE, Rgba::BLUE, Rgba::YELLOW, Rgba::YELLOW });
const Pattern_t EscapeWorld::s_singleMonitorPattern(2, 2, { Rgba::GREEN, Rgba::GREEN, Rgba::GREEN, Rgba::RED });
const Pattern_t EscapeWorld::s_scanAllMonitorsPattern(2, 2, { Rgba::RED, Rgba::RED, Rgba::RED, Rgba::RED });
const Pattern_t EscapeWorld::s_motionDetectorPattern(2, 2, { Rgba::YELLOW, Rgba::YELLOW, Rgba::GREEN, Rgba::GREEN });


const IntVector2 EscapeWorld::s_monitorSpriteCoords(15, 23);
const IntVector2 EscapeWorld::s_unkownSpriteCoords(1, 2);
const IntVector2 EscapeWorld::s_treasureSpriteCoords(15, 22);
const IntVector2 EscapeWorld::s_treasureOpenSpriteCoords(16, 22);
const IntVector2 EscapeWorld::s_guardCursorSpriteCoords(1, 2);
const IntVector2 Thief::s_spriteCoords(16, 21);
const IntVector2 Guard::s_spriteCoords(15, 21);


const AABB2 EscapeWorld::s_playableAreaBounds(Vector2(4, 0), Vector2(42, 19));
const AABB2 EscapeWorld::s_infoAreaBounds(Vector2(0, 19), Vector2(42, 23));
const AABB2 EscapeWorld::s_actionAreaBounds(Vector2(0, 0), Vector2(4, 19));
const AABB2 EscapeWorld::s_actionItemSlotBounds(Vector2(1, 11), Vector2(3, 13));
const AABB2 EscapeWorld::s_equipmentSlotBounds(Vector2(1, 4), Vector2(3, 6));

const Pattern_t Thief::s_legoPattern(1, 1, { Rgba::RED });
const Pattern_t Guard::s_legoPattern(1, 1, { Rgba::BLUE });

int			Thief::s_maxMoveDistance;
IntRange	Guard::s_moveDistanceRange;
int			Guard::s_flashlightFullBattery;
float		Guard::s_possibilityForSingleMonitor;
float		Guard::s_possibilityForScanAllMonitors;
float		Guard::s_possibilityForMotionDetector;
int			Guard::s_alertDistanceDefault;
int			Guard::s_alertDistanceWithFlashlight;

EscapeWorld::EscapeWorld() {
	InitBoard();
	InitCameras();
	LoadGameConfigData();
	LoadDefaultResources();
	TileDefinition::LoadAllDefinitions();
	LoadMap("Data/TiledMap/Escape.tmx");
}

EscapeWorld::~EscapeWorld() {
// 	for (auto& it : TileDefinition::s_definitions) {
// 		delete it.second;
// 	}
}

void EscapeWorld::InitBoard() {
	m_board = std::make_unique<Board>();
}

void EscapeWorld::InitCameras() {
	// Create the Thief screen camera
	m_thiefCamera = std::make_unique<Camera>();
	m_thiefCamera->SetViewport(0U, 0, 0, g_thiefOutput->GetWidth(), g_thiefOutput->GetHeight(), 0.f, 1.f);
	m_thiefCamera->SetProjectionMode(ORTHOGRAPHIC);
	m_thiefCamera->SetOrtho(Vector2::ZERO, Vector2(g_thiefOutput->GetWidth(), g_thiefOutput->GetHeight()), 0.f, 10.f);

	// Create the Board window camera
	m_boardCamera = std::make_unique<Camera>();
	m_boardCamera->SetViewport(0U, 0, 0, g_boardOutput->GetWidth(), g_boardOutput->GetHeight(), 0.f, 1.f);
	m_boardCamera->SetProjectionMode(ORTHOGRAPHIC);
	m_boardCamera->SetOrtho(Vector2::ZERO, Vector2(g_boardOutput->GetWidth(), g_boardOutput->GetHeight()), 0.f, 10.f);

	// Setup debug draw system to use my 2d/3d cameras
	g_theDebugDrawSystem->SetCamera2D(m_boardCamera.get());
}

void EscapeWorld::LoadGameConfigData() {
	pugi::xml_document doc;
	doc.load_file("Data/GameConfig.xml");

	m_gameConfig = std::make_unique<Blackboard>();
	m_gameConfig->PopulateFromXmlElementAttributes(doc);

	m_totalMonitors = m_gameConfig->GetValue("TotalMonitors", 0);
	m_totalTreasures = m_gameConfig->GetValue("TotalTreasures", 0);
	m_leastTreasuresStolenToWin = m_gameConfig->GetValue("LeastTreasuresStolenToWin", 0);
	m_monitorAlertDistanceDefault = m_gameConfig->GetValue("MonitorAlertDistanceDefault", 0);

	
	m_gridWidthInThiefWindow = g_thiefOutput->GetWidth() / (float)GRID_COUNT_X;
	m_gridHeightInThiefWindow = g_thiefOutput->GetHeight() / (float)GRID_COUNT_Y;
	m_widthInThiefWindow = g_thiefOutput->GetWidth();
	m_heightInThiefWindow = g_thiefOutput->GetHeight();

	// Load rooms
	pugi::xml_node roomsNode = doc.child("Rooms");
	for (pugi::xml_node node : roomsNode.children()) {
		if (std::string(node.name()) == "Box") {
			std::vector<IntVector2> grids;
			IntVector2 mins; 
			mins.SetFromText(node.attribute("mins").as_string());
			IntVector2 maxs;
			maxs.SetFromText(node.attribute("maxs").as_string());
			for (int y = mins.y; y < maxs.y; ++y) {
				for (int x = mins.x; x < maxs.x; ++x) {
					grids.emplace_back(x, y);
				}
			}
			m_rooms.push_back(grids);
		}
		else if (std::string(node.name()) == "Grids") {
			std::vector<IntVector2> grids;
			for (pugi::xml_node gridNode : node.children()) {
				IntVector2 value;
				value.SetFromText(gridNode.attribute("value").as_string());
				grids.push_back(value);
			}
			m_rooms.push_back(grids);
		}
	}

}

void EscapeWorld::LoadDefaultResources() {
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

void EscapeWorld::LoadMap(const std::string& filePath) {
	m_tileSet = g_theResourceManager->GetSpriteSheet("terrain_32x32");

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filePath.c_str());
	if (!result) {
		ERROR_AND_DIE(Stringf("Game[%s]: Map file doesn't exist!", filePath.c_str()));
	}

	pugi::xml_node mapNode = doc.child("map");
	//pugi::xml_node tilesetNode = mapNode.child("tileset");
	pugi::xml_node layerNode = mapNode.child("layer");
	pugi::xml_node dataNode = layerNode.child("data");
	std::string csvText = dataNode.text().as_string();
	std::stringstream ss(csvText);

	// Read .csv data
	for (int j = GRID_COUNT_Y - 1; j >= 0; --j) {
		for (int i = 0; i < GRID_COUNT_X; ++i) {
			int indexInTileset;
			ss >> indexInTileset;

			// TiledMap starts with index 1, 0 is invalid, so should subtract 1 here
			if (indexInTileset > 0) {
				indexInTileset -= 1;
				int index = j * GRID_COUNT_X + i;
				m_tiles[index].m_indexInTileset = indexInTileset;
				IntVector2 spriteCoords(indexInTileset % 32, indexInTileset / 32);
				m_tiles[index].m_spriteCoords = spriteCoords;

				TileDefinition* tileDef = TileDefinition::GetDefinition(indexInTileset);
				m_tiles[index].m_isEntrance = tileDef->m_isEntrance;
				m_tiles[index].m_isOpaque = tileDef->m_isOpaque;
				m_tiles[index].m_isSolid = tileDef->m_isSolid;
			}

			if (ss.peek() == ',' || ss.peek() == ' ') {
				ss.ignore();
			}
		}

	}
}

RaycastResult2D_t EscapeWorld::Raycast(const Vector2& startPos, const Vector2& direction, float maxDistance) const {
	RaycastResult2D_t raycastResult;

	IntVector2 previousTileCoords(RoundToNearestInt(startPos.x / GRID_WIDTH_IN_BOARD_WINDOW), RoundToNearestInt(startPos.y / GRID_HEIGHT_IN_BOARD_WINDOW));

	IntVector2 currentTileCoords = IntVector2();

	Vector2	currentPos;

	Vector2	singleStep = direction / static_cast<float>(RAYCAST_STEPS_PER_Unit);

	int numSteps = static_cast<int>(maxDistance) * RAYCAST_STEPS_PER_Unit;

	for (int stepNumber = 0; stepNumber <= numSteps; ++stepNumber) {
		currentPos = startPos + (singleStep * static_cast<float>(stepNumber));
		currentTileCoords = IntVector2((int)floorf(currentPos.x / GRID_WIDTH_IN_BOARD_WINDOW), (int)floorf(currentPos.y / GRID_HEIGHT_IN_BOARD_WINDOW));

		if (currentTileCoords != previousTileCoords) {
			int tileIndex = currentTileCoords.x + currentTileCoords.y * GRID_COUNT_X;
			bool isOpaque = m_tiles[tileIndex].m_isOpaque;

			if (isOpaque) {
				//Have an impact

				//Fill out RaycastResult
				raycastResult.m_didImpact = true;
				raycastResult.m_impactPos = currentPos;
				raycastResult.m_impactTileCoords = currentTileCoords;
				raycastResult.m_impactDistance = ((float)stepNumber * singleStep).GetLength();
				raycastResult.m_impactFraction = GetFractionInRange(raycastResult.m_impactDistance, 0.f, maxDistance);
				raycastResult.m_impactSurfaceNormal = Vector2((previousTileCoords - currentTileCoords).x, (previousTileCoords - currentTileCoords).y);

				return raycastResult;
			}
			else {
				previousTileCoords = currentTileCoords;
			}
		}
	}
	//Reach the end of the for-loop, fill out raycastResult
	raycastResult.m_didImpact = false;
	raycastResult.m_impactPos = currentPos;
	raycastResult.m_impactTileCoords = currentTileCoords;
	raycastResult.m_impactDistance = maxDistance;

	return raycastResult;
}

RaycastResult2D_t EscapeWorld::RaycastOnThiefWindow(const Vector2& startPos, const Vector2& direction, float maxDistance) const {
	RaycastResult2D_t raycastResult;

	IntVector2 previousTileCoords(RoundToNearestInt(startPos.x / m_gridWidthInThiefWindow), RoundToNearestInt(startPos.y / m_gridHeightInThiefWindow));

	IntVector2 currentTileCoords = IntVector2();

	Vector2	currentPos;

	Vector2	singleStep = direction / static_cast<float>(RAYCAST_STEPS_PER_Unit);

	int numSteps = static_cast<int>(maxDistance) * RAYCAST_STEPS_PER_Unit;

	for (int stepNumber = 0; stepNumber <= numSteps; ++stepNumber) {
		currentPos = startPos + (singleStep * static_cast<float>(stepNumber));
		currentTileCoords = IntVector2((int)floorf(currentPos.x / m_gridWidthInThiefWindow), (int)floorf(currentPos.y / m_gridHeightInThiefWindow));

		if (currentTileCoords != previousTileCoords) {
			int tileIndex = currentTileCoords.x + currentTileCoords.y * GRID_COUNT_X;
			bool isOpaque = m_tiles[tileIndex].m_isOpaque;

			if (isOpaque) {
				//Have an impact

				//Fill out RaycastResult
				raycastResult.m_didImpact = true;
				raycastResult.m_impactPos = currentPos;
				raycastResult.m_impactTileCoords = currentTileCoords;
				raycastResult.m_impactDistance = ((float)stepNumber * singleStep).GetLength();
				raycastResult.m_impactFraction = GetFractionInRange(raycastResult.m_impactDistance, 0.f, maxDistance);
				raycastResult.m_impactSurfaceNormal = Vector2((previousTileCoords - currentTileCoords).x, (previousTileCoords - currentTileCoords).y);

				return raycastResult;
			}
			else {
				previousTileCoords = currentTileCoords;
			}
		}
	}
	//Reach the end of the for-loop, fill out raycastResult
	raycastResult.m_didImpact = false;
	raycastResult.m_impactPos = currentPos;
	raycastResult.m_impactTileCoords = currentTileCoords;
	raycastResult.m_impactDistance = maxDistance;

	return raycastResult;
}

float EscapeWorld::GetDepthValueAtGridCoords(const IntVector2& gridCoords) {
	BoardData_t* data = m_board->m_boardData.Read();
	if (!data) {
		return 0.f;
	}
	int gridIndex = gridCoords.y * GRID_COUNT_X + gridCoords.x;
	return data->depthValues[gridIndex];
}

void EscapeWorld::FindPatternInArea(const Pattern_t& pattern, const AABB2& bounds, std::vector<IntVector2>& out) {
	bool visited[GRID_COUNT_TOTAL] = { false };
	BoardData_t* data = m_board->m_boardData.Read();
	if (!data) {
		return;
	}
	for (int gridY = bounds.mins.y; gridY <= bounds.maxs.y - (pattern.m_height - 1); ++gridY) {
		for (int gridX = bounds.mins.x; gridX <= bounds.maxs.x - (pattern.m_width - 1); ++gridX) {
			int gridIndex = gridY * GRID_COUNT_X + gridX;
			if (visited[gridIndex]) {
				continue;
			}

			if (m_tiles[gridIndex].m_isSolid) {
				continue;
			}

			Rgba color = data->GetColorAtGridCoords(gridX, gridY);
			bool flag = true;
			for (int y = 0; y < pattern.m_height; ++y) {
				for (int x = 0; x < pattern.m_width; ++x) {
					Rgba c = data->GetColorAtGridCoords(gridX + x, gridY + y);
					int patternIdx = y * pattern.m_width + x;
					Rgba patternColor = pattern.m_colors[patternIdx];
					if (!c.RgbEqual(patternColor)) {
						flag = false;
						break;
					}
				}
			}
			// find pattern succeed
			if (flag) {
				out.emplace_back(gridX, gridY);
				for (int y = 0; y < pattern.m_height; ++y) {
					for (int x = 0; x < pattern.m_width; ++x) {
						int gridIndex = (gridY + y) * GRID_COUNT_X + (gridX + x);
						visited[gridIndex] = true;
					}
				}
			}
		}
	}
}

bool EscapeWorld::IsPatternThere(const Pattern_t& pattern, const IntVector2& gridCoords) {
	BoardData_t* data = m_board->m_boardData.Read();
	if (!data) {
		return false;
	}
	Rgba color = data->GetColorAtGridCoords(gridCoords.x, gridCoords.y);
	for (int y = 0; y < pattern.m_height; ++y) {
		for (int x = 0; x < pattern.m_width; ++x) {
			Rgba c = data->GetColorAtGridCoords(gridCoords.x + x, gridCoords.y + y);
			int patternIdx = y * pattern.m_width + x;
			Rgba patternColor = pattern.m_colors[patternIdx];
			if (!c.RgbEqual(patternColor)) {
				return false;
			}
		}
	}
	return true;
}

void EscapeWorld::FindMovableGridsWithinRange(const IntVector2& startCoords, int range, std::vector<IntVector2>& out) {
	out.clear();

	std::array<int, GRID_COUNT_TOTAL> distances;
	std::fill(distances.begin(), distances.end(), 9999);

	int startIndex = startCoords.y * GRID_COUNT_X + startCoords.x;
	distances[startIndex] = 0;

	std::queue<IntVector2> openQueue;
	openQueue.push(startCoords);

	while (!openQueue.empty()) {
		IntVector2 thisCoords = openQueue.front();
		openQueue.pop();

		int thisIndex = thisCoords.y * GRID_COUNT_X + thisCoords.x;
		int thisDistance = distances[thisIndex];

		IntVector2 eastCoords(thisCoords.x + 1, thisCoords.y);
		IntVector2 westCoords(thisCoords.x - 1, thisCoords.y);
		IntVector2 northCoords(thisCoords.x, thisCoords.y + 1);
		IntVector2 southCoords(thisCoords.x, thisCoords.y - 1);

		if (0 <= eastCoords.x && eastCoords.x < GRID_COUNT_X && 0 <= eastCoords.y && eastCoords.y < GRID_COUNT_Y) {
			if (!IsGuardThere(eastCoords)) {
				int eastIndex = eastCoords.y * GRID_COUNT_X + eastCoords.x;
				bool isSolid = m_tiles[eastIndex].m_isSolid;
				if (!isSolid) {
					int eastDistance = distances[eastIndex];
					if (eastDistance > thisDistance + 1) {
						distances[eastIndex] = thisDistance + 1;
						openQueue.push(eastCoords);
					}
				}
			}
		}

		if (0 <= westCoords.x && westCoords.x < GRID_COUNT_X && 0 <= westCoords.y && westCoords.y < GRID_COUNT_Y) {
			if (!IsGuardThere(westCoords)) {
				int westIndex = westCoords.y * GRID_COUNT_X + westCoords.x;
				bool isSolid = m_tiles[westIndex].m_isSolid;
				if (!isSolid) {
					int westDistance = distances[westIndex];
					if (westDistance > thisDistance + 1) {
						distances[westIndex] = thisDistance + 1;
						openQueue.push(westCoords);
					}
				}
			}
		}

		if (0 <= northCoords.x && northCoords.x < GRID_COUNT_X && 0 <= northCoords.y && northCoords.y < GRID_COUNT_Y) {
			if (!IsGuardThere(northCoords)) {
				int northIndex = northCoords.y * GRID_COUNT_X + northCoords.x;
				bool isSolid = m_tiles[northIndex].m_isSolid;
				if (!isSolid) {
					int northDistance = distances[northIndex];
					if (northDistance > thisDistance + 1) {
						distances[northIndex] = thisDistance + 1;
						openQueue.push(northCoords);
					}
				}
			}
		}

		if (0 <= southCoords.x && southCoords.x < GRID_COUNT_X && 0 <= southCoords.y && southCoords.y < GRID_COUNT_Y) {
			if (!IsGuardThere(southCoords)) {
				int southIndex = southCoords.y * GRID_COUNT_X + southCoords.x;
				bool isSolid = m_tiles[southIndex].m_isSolid;
				if (!isSolid) {
					int southDistance = distances[southIndex];
					if (southDistance > thisDistance + 1) {
						distances[southIndex] = thisDistance + 1;
						openQueue.push(southCoords);
					}
				}
			}
		}
	}
	for (int i = 0; i < GRID_COUNT_TOTAL; ++i) {
		if (0 < distances[i] && distances[i] <= range) {
			IntVector2 gridCoords(i % GRID_COUNT_X, i / GRID_COUNT_X);
			out.push_back(gridCoords);
		}
	}
}

bool EscapeWorld::IsGuardThere(const IntVector2& gridCoords) {
	for (int i = 0; i < m_guards.size(); ++i) {
		const IntVector2& guardGridCoords = m_guards[i].m_gridCoords;
		if (guardGridCoords == gridCoords) {
			return true;
		}
	}
	return false;
}

bool EscapeWorld::CheckIfThiefIsInRoom(std::vector<IntVector2>& out) {
	for (auto r : m_rooms) {
		for (auto gridCoords : r) {
			if (gridCoords == m_thief.m_gridCoords) {
				for (auto grid : r) {
					out.push_back(grid);
				}
				return true;
			}
		}
	}
	return false;
}

Vector2 EscapeWorld::GetGridPositionFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const {
	return Vector2(GRID_WIDTH_IN_BOARD_WINDOW * (float)gridCoords.x, GRID_HEIGHT_IN_BOARD_WINDOW * (float)gridCoords.y);
}

Vector2 EscapeWorld::GetGridPositionFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const {
	return Vector2(m_gridWidthInThiefWindow * (float)gridCoords.x, m_gridWidthInThiefWindow * (float)gridCoords.y);
}

Vector2 EscapeWorld::GetGridCenterFromGridCoordsOnBoardWindow(const IntVector2& gridCoords) const {
	return GetGridPositionFromGridCoordsOnBoardWindow(gridCoords) + 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
}

Vector2 EscapeWorld::GetGridCenterFromGridCoordsOnThiefWindow(const IntVector2& gridCoords) const {
	return GetGridPositionFromGridCoordsOnThiefWindow(gridCoords) + 0.5f * Vector2(m_gridWidthInThiefWindow, m_gridHeightInThiefWindow);
}

std::string EscapeWorld::GetStringFromActionType(eActionType actionType) const {
	switch (actionType) {
	case ACTION_TYPE_SINGLEMONITOR:		return "Check Single Monitor";
	case ACTION_TYPE_SCANALLMONITORS:	return "Scan All Monitors";
	case ACTION_TYPE_MOTIONDETECTOR:	return "Activate Motion Detector";
	default:							return "invalid";
	}
}

bool EscapeWorld::CheckIfMonitorCanSeeThief(const IntVector2& monitorGridCoords) {
	Vector2 monitorCenter = GetGridCenterFromGridCoordsOnBoardWindow(monitorGridCoords);
	Vector2 bl_monitor = monitorCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 br_monitor = monitorCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 tl_monitor = monitorCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 tr_monitor = monitorCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

	Vector2 thiefCenter = GetGridCenterFromGridCoordsOnBoardWindow(m_thief.m_gridCoords);
	Vector2 bl_thief = thiefCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 br_thief = thiefCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, -GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 tl_thief = thiefCenter + 0.8 * 0.5f * Vector2(-GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);
	Vector2 tr_thief = thiefCenter + 0.8 * 0.5f * Vector2(GRID_WIDTH_IN_BOARD_WINDOW, GRID_HEIGHT_IN_BOARD_WINDOW);

	Vector2 bl_dir = (bl_thief - bl_monitor).GetNormalized();
	Vector2 br_dir = (br_thief - br_monitor).GetNormalized();
	Vector2 tl_dir = (tl_thief - tl_monitor).GetNormalized();
	Vector2 tr_dir = (tr_thief - tr_monitor).GetNormalized();

	float bl_distance = GetDistance(bl_thief, bl_monitor);
	float br_distance = GetDistance(br_thief, br_monitor);
	float tl_distance = GetDistance(tl_thief, tl_monitor);
	float tr_distance = GetDistance(tr_thief, tr_monitor);

	RaycastResult2D_t bl_result = Raycast(bl_monitor, bl_dir, bl_distance);
	RaycastResult2D_t br_result = Raycast(br_monitor, br_dir, br_distance);
	RaycastResult2D_t tl_result = Raycast(tl_monitor, tl_dir, tl_distance);
	RaycastResult2D_t tr_result = Raycast(tr_monitor, tr_dir, tr_distance);

	bool result = false;
	m_thief.m_fractionOfExposion = 0.f;
	if (!bl_result.m_didImpact) {
		m_thief.m_fractionOfExposion += 0.25f;
		result = true;
	}
	if (!br_result.m_didImpact) {
		m_thief.m_fractionOfExposion += 0.25f;
		result = true;
	}
	if (!tl_result.m_didImpact) {
		m_thief.m_fractionOfExposion += 0.25f;
		result = true;
	}
	if (!tr_result.m_didImpact) {
		m_thief.m_fractionOfExposion += 0.25f;
		result = true;
	}

	if (result) {
		float distanceOfCoords = (Vector2(monitorGridCoords) - Vector2(m_thief.m_gridCoords)).GetLength();
		if (distanceOfCoords <= 3.f || monitorGridCoords.x == m_thief.m_gridCoords.x || monitorGridCoords.y == m_thief.m_gridCoords.y) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

