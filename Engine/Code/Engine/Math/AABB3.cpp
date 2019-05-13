#include "Engine/Math/AABB3.hpp"

AABB3::AABB3(const Vector3& min, const Vector3& max)
	: mins(min)
	, maxs(max) {}

bool AABB3::IsPointInside(const Vector3& point) {
	if (point.x > mins.x && point.x < maxs.x &&
		point.y > mins.y && point.y < maxs.y &&
		point.z > mins.z && point.z < maxs.z) {
		return true;
	}
	else {
		return false;
	}
}

void AABB3::StretchToIncludePoint(const Vector3& point) {
	if (point.x < mins.x) mins.x = point.x;
	if (point.x > maxs.x) maxs.x = point.x;
	if (point.y < mins.y) mins.y = point.y;
	if (point.y > maxs.y) maxs.y = point.y;
	if (point.z < mins.z) mins.z = point.z;
	if (point.z > maxs.z) maxs.z = point.z;
}

