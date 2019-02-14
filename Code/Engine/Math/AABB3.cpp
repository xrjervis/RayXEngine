#include "Engine/Math/AABB3.hpp"

AABB3::AABB3(const Vector3& center, const Vector3& halfVector) 
	: m_center(center)
	, m_halfVector(halfVector) {}

bool AABB3::IsPointInside(const Vector3& point) {
	auto max = m_center + m_halfVector;
	auto min = m_center - m_halfVector;

	if (point.x > min.x && point.x < max.x &&
		point.y > min.y && point.y < max.y &&
		point.z > min.z && point.z < max.z) {
		return true;
	}
	else {
		return false;
	}
}

