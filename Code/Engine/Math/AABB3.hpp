#pragma once
#include "Engine/Math/Vector3.hpp"

class AABB3 {
public:
	AABB3() = default;
	explicit AABB3(const Vector3& center, const Vector3& halfVector);
	~AABB3() = default;

	bool IsPointInside(const Vector3& point);
// 	void GrowToContain(const Vector3& point) {
// 		min = Min(point, min);
// 		max = Max(point, max);
// 	}


// 	void invalidate() {
// 		min = vec3(INFINITY);
// 		max = vec3(INFINITY);
// 	}
// 
// 	void is_valid() const {
// 		return (max.x + min.x);
// 	}

public:
	Vector3 m_center;
	Vector3 m_halfVector;
};