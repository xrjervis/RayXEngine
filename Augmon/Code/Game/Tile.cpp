#include "Game/Tile.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/EngineCommon.hpp"

std::map<int, TileDefinition*> TileDefinition::s_definitions;

void TileDefinition::LoadAllDefinitions() {
	TileDefinition* tileDef_Wall = new TileDefinition();
	tileDef_Wall->m_isEntrance = false;
	tileDef_Wall->m_isOpaque = true;
	tileDef_Wall->m_isSolid = true;

	TileDefinition* tileDef_Entrance = new TileDefinition();
	tileDef_Entrance->m_isEntrance = true;
	tileDef_Entrance->m_isOpaque = false;
	tileDef_Entrance->m_isSolid = false;

	TileDefinition* tileDef_Floor = new TileDefinition();
	tileDef_Floor->m_isEntrance = false;
	tileDef_Floor->m_isOpaque = false;
	tileDef_Floor->m_isSolid = false;

	TileDefinition* tileDef_Water = new TileDefinition();
	tileDef_Water->m_isEntrance = false;
	tileDef_Water->m_isOpaque = false;
	tileDef_Water->m_isSolid = true;

	s_definitions.insert({ 225, tileDef_Wall });
	s_definitions.insert({ 7, tileDef_Wall });
	s_definitions.insert({ 30, tileDef_Floor});
	s_definitions.insert({ 411, tileDef_Floor });
	s_definitions.insert({ 125, tileDef_Entrance });
	s_definitions.insert({ 371, tileDef_Entrance });

	s_definitions.insert({ 716, tileDef_Water });
	s_definitions.insert({ 717, tileDef_Water });
	s_definitions.insert({ 718, tileDef_Water });

	s_definitions.insert({ 748, tileDef_Water });
	s_definitions.insert({ 953, tileDef_Water });
	s_definitions.insert({ 750, tileDef_Water });

	s_definitions.insert({ 780, tileDef_Water });
	s_definitions.insert({ 781, tileDef_Water });
	s_definitions.insert({ 782, tileDef_Water });
}

TileDefinition* TileDefinition::GetDefinition(int tileIndex) {
	if (s_definitions.find(tileIndex) != s_definitions.end()) {
		return s_definitions.at(tileIndex);
	}
	return nullptr;
}
