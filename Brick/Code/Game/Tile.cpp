#include "Game/Tile.hpp"

Tile::Tile(SpriteSheet* ss, int index) 
	: m_tileSet(ss)
	, m_indexInTileSet(index) {

}

Tile::~Tile() {
	m_tileSet = nullptr;
}
