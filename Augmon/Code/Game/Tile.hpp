#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <map>


class TileDefinition {
public:
	TileDefinition() = default;
	~TileDefinition() = default;

	static void LoadAllDefinitions();
	static TileDefinition* GetDefinition(int tileIndex);


public:
	bool										m_isSolid;
	bool										m_isOpaque;
	bool										m_isEntrance;
	static std::map<int, TileDefinition*>		s_definitions;
};

class Tile {
public:
	Tile() = default;
	~Tile() = default;

public:
	bool		m_isSolid = true;
	bool		m_isOpaque = false;
	bool		m_isEntrance = false;
	int			m_indexInTileset;
	IntVector2	m_spriteCoords;
};
