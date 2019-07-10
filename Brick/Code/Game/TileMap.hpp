#pragma once
#include "Game/Tile.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>

class TileMap {
public:
	explicit TileMap(const IntVector2& dim);

public:
	IntVector2			m_dimension;
	std::vector<Tile>	m_tiles;
};