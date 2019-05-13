#pragma once
#include "Engine/Math/Vector3.hpp"

class AABB3 {
public:
	AABB3() = default;
	explicit AABB3(const Vector3& min, const Vector3& max);
	~AABB3() = default;

	bool IsPointInside(const Vector3& point);
	void StretchToIncludePoint(const Vector3& point);

public:
	Vector3 mins;
	Vector3 maxs;
};