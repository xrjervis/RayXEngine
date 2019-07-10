#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class SpriteSheet;

class Tile {
public:
	explicit Tile(SpriteSheet* ss, int index);
	~Tile();

public:
	SpriteSheet*		m_tileSet = nullptr;
	int					m_indexInTileSet;
};