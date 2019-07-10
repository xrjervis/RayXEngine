#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>

class CollisionMap {
public:
	CollisionMap(const IntVector2& dim);
	~CollisionMap() = default;

	void AppendAABB2AtIndex(const AABB2& aabb2, int index);
	void AppendAABB2AtCoords(const AABB2& aabb2, int x, int y);
	void AppendAABB2AtCoords(const AABB2& aabb2, const IntVector2& coords);

public:
	IntVector2							m_dimension;
	std::vector<std::vector<AABB2>>		m_aabb2Colliders;
};