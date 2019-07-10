#include "Game/CollisionMap.hpp"

CollisionMap::CollisionMap(const IntVector2& dim) {
	m_dimension = dim;
	m_aabb2Colliders.reserve(m_dimension.x * m_dimension.y);
}

void CollisionMap::AppendAABB2AtIndex(const AABB2& aabb2, int index) {
	m_aabb2Colliders.at(index).emplace_back(aabb2);
}

void CollisionMap::AppendAABB2AtCoords(const AABB2& aabb2, int x, int y) {
	int index = y * m_dimension.x + x;
	AppendAABB2AtIndex(aabb2, index);
}

void CollisionMap::AppendAABB2AtCoords(const AABB2& aabb2, const IntVector2& coords) {
	AppendAABB2AtCoords(aabb2, coords.x, coords.y);
}
