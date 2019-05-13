#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/type.hpp"
#include <string>
#include <array>
#include <vector>

enum eBLockType : int {
	BLOCK_TYPE_AIR = 0,
	BLOCK_TYPE_UNKOWN,
	BLOCK_TYPE_TEST,
	BLOCK_TYPE_STONE,
	BLOCK_TYPE_DIRT,
	BLOCK_TYPE_GRASS,
	BLOCK_TYPE_ICE,
	BLOCK_TYPE_COBBLESTONE,
	BLOCK_TYPE_SAND,
	BLOCK_TYPE_GLOWSTONE,
	NUM_BLOCK_TYPES
};

class BlockDefinition {
public:
	~BlockDefinition() = default;
	BlockDefinition() = default;

public:
	IntVector2	m_topSpriteCoords;
	IntVector2	m_bottomSpriteCoords;
	IntVector2	m_sideSpriteCoords;
	u8			m_bitFlags = 0U;
	u8			m_indoorLightLevel = 0U;

	static void				LoadDefinitions();
	static BlockDefinition* GetDefinition(eBLockType type);

	static std::array<BlockDefinition, NUM_BLOCK_TYPES> s_definitions;
};

constexpr u8 BLOCK_BIT_IS_SKY = 0b10000000;
constexpr u8 BLOCK_BIT_IS_LIGHT_DIRTY = 0b01000000;
constexpr u8 BLOCK_BIT_IS_FULL_OPAQUE = 0b00100000;
constexpr u8 BLOCK_BIT_IS_SOLID = 0b00010000;
constexpr u8 BLOCK_BIT_IS_VISIBLE = 0b00001000;

constexpr u8 BLOCK_INDOOR_LIGHT_MASK = 0b00001111;
constexpr u8 BLOCK_OUTDOOR_LIGHT_MASK = 0b11110000;

class Block {
public:
	Block();
	~Block();

	u8 GetIndoorLightLevel() const;
	u8 GetOutdoorLightLevel() const;

	void SetIndoorLightLevel(u8 level);
	void SetOutDoorLightLevel(u8 level);

	bool IsBlockSky() const;
	void SetIsBlockSky();
	void ClearIsBlockSky();

	bool IsLightDirty() const;
	void SetIsLightDirty();
	void ClearIsLightDirty();

	bool IsFullOpaque() const;
	void SetIsFullOpaque();
	void ClearIsFullOpaque();

	bool IsSolid() const;
	void SetIsSolid();
	void ClearIsSolid();

	eBLockType	m_type;
	u8			m_lightLevel; // high (4) bits are outdoor light; low (4) bits are indoor light
	u8			m_bitFlags;
};

inline u8 Block::GetIndoorLightLevel() const {
	return (m_lightLevel & BLOCK_INDOOR_LIGHT_MASK);
}

inline u8 Block::GetOutdoorLightLevel() const {
	return ((m_lightLevel & BLOCK_OUTDOOR_LIGHT_MASK) >> 4);
}

inline void Block::SetIndoorLightLevel(u8 level) {
	m_lightLevel = (level) + (m_lightLevel & BLOCK_OUTDOOR_LIGHT_MASK);
}

inline void Block::SetOutDoorLightLevel(u8 level) {
	m_lightLevel = (level << 4) + (m_lightLevel & BLOCK_INDOOR_LIGHT_MASK);
}

inline bool Block::IsBlockSky() const{
	return AreBitsSet(m_bitFlags, BLOCK_BIT_IS_SKY);
}

inline void Block::SetIsBlockSky() {
	SetBits(m_bitFlags, BLOCK_BIT_IS_SKY);
}

inline void Block::ClearIsBlockSky() {
	ClearBits(m_bitFlags, BLOCK_BIT_IS_SKY);
}

inline bool Block::IsLightDirty() const {
	return AreBitsSet(m_bitFlags, BLOCK_BIT_IS_LIGHT_DIRTY);
}

inline void Block::SetIsLightDirty() {
	SetBits(m_bitFlags, BLOCK_BIT_IS_LIGHT_DIRTY);
}

inline void Block::ClearIsLightDirty() {
	ClearBits(m_bitFlags, BLOCK_BIT_IS_LIGHT_DIRTY);
}

inline bool Block::IsFullOpaque() const {
	return AreBitsSet(m_bitFlags, BLOCK_BIT_IS_FULL_OPAQUE);
}

inline void Block::SetIsFullOpaque() {
	SetBits(m_bitFlags, BLOCK_BIT_IS_FULL_OPAQUE);
}

inline void Block::ClearIsFullOpaque() {
	ClearBits(m_bitFlags, BLOCK_BIT_IS_FULL_OPAQUE);
}

inline bool Block::IsSolid() const {
	return AreBitsSet(m_bitFlags, BLOCK_BIT_IS_SOLID);
}

inline void Block::SetIsSolid() {
	SetBits(m_bitFlags, BLOCK_BIT_IS_SOLID);
}

inline void Block::ClearIsSolid() {
	ClearBits(m_bitFlags, BLOCK_BIT_IS_SOLID);
}
