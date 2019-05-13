#include "Game/Block.hpp"

void BlockDefinition::LoadDefinitions() {
	// Air
	BlockDefinition* airDef = GetDefinition(BLOCK_TYPE_GRASS);
	SetBits(airDef->m_bitFlags, 0b00000000);

	// Grass
	BlockDefinition* grassDef = GetDefinition(BLOCK_TYPE_GRASS);
	grassDef->m_topSpriteCoords = IntVector2(1, 0);
	grassDef->m_sideSpriteCoords = IntVector2(3, 3);
	grassDef->m_bottomSpriteCoords = IntVector2(4, 3);
	SetBits(grassDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Stone
	BlockDefinition* stoneDef = GetDefinition(BLOCK_TYPE_COBBLESTONE);
	stoneDef->m_topSpriteCoords = IntVector2(0, 5);
	stoneDef->m_sideSpriteCoords = IntVector2(0, 5);
	stoneDef->m_bottomSpriteCoords = IntVector2(0, 5);
	SetBits(stoneDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Dirt
	BlockDefinition* dirtDef = GetDefinition(BLOCK_TYPE_DIRT);
	dirtDef->m_topSpriteCoords = IntVector2(4, 3);
	dirtDef->m_sideSpriteCoords = IntVector2(4, 3);
	dirtDef->m_bottomSpriteCoords = IntVector2(4, 3);
	SetBits(dirtDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Ice
	BlockDefinition* iceDef = GetDefinition(BLOCK_TYPE_ICE);
	iceDef->m_topSpriteCoords = IntVector2(1, 3);
	iceDef->m_sideSpriteCoords = IntVector2(1, 3);
	iceDef->m_bottomSpriteCoords = IntVector2(1, 3);
	SetBits(iceDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Stone
	BlockDefinition* lavaDef = GetDefinition(BLOCK_TYPE_STONE);
	lavaDef->m_topSpriteCoords = IntVector2(7, 4);
	lavaDef->m_sideSpriteCoords = IntVector2(7, 4);
	lavaDef->m_bottomSpriteCoords = IntVector2(7, 4);
	SetBits(lavaDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Sand
	BlockDefinition* sandDef = GetDefinition(BLOCK_TYPE_SAND);
	sandDef->m_topSpriteCoords = IntVector2(6, 1);
	sandDef->m_sideSpriteCoords = IntVector2(6, 1);
	sandDef->m_bottomSpriteCoords = IntVector2(6, 1);
	SetBits(sandDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Test
	BlockDefinition* testDef = GetDefinition(BLOCK_TYPE_TEST);
	testDef->m_topSpriteCoords = IntVector2(31, 31);
	testDef->m_sideSpriteCoords = IntVector2(31, 31);
	testDef->m_bottomSpriteCoords = IntVector2(31, 31);
	SetBits(testDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

	// Unknown
	BlockDefinition* unknownDef = GetDefinition(BLOCK_TYPE_UNKOWN);
	unknownDef->m_topSpriteCoords = IntVector2(1, 2);
	unknownDef->m_sideSpriteCoords = IntVector2(1, 2);
	unknownDef->m_bottomSpriteCoords = IntVector2(1, 2);
	SetBits(unknownDef->m_bitFlags, BLOCK_BIT_IS_SOLID);

	// GlowStone
	BlockDefinition* glowStoneDef = GetDefinition(BLOCK_TYPE_GLOWSTONE);
	glowStoneDef->m_topSpriteCoords = IntVector2(3, 13);
	glowStoneDef->m_sideSpriteCoords = IntVector2(3, 13);
	glowStoneDef->m_bottomSpriteCoords = IntVector2(3, 13);
	glowStoneDef->m_indoorLightLevel = 12;
	SetBits(glowStoneDef->m_bitFlags, BLOCK_BIT_IS_SOLID | BLOCK_BIT_IS_FULL_OPAQUE);

}

BlockDefinition* BlockDefinition::GetDefinition(eBLockType type) {
	return &s_definitions[type];
}

std::array<BlockDefinition, NUM_BLOCK_TYPES> BlockDefinition::s_definitions;

Block::Block() {
	
}

Block::~Block() {

}