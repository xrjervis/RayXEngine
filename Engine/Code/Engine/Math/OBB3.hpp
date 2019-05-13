#pragma once
#include "Engine/Math/Vector3.hpp"

class OBB3 {
public:
	OBB3() {}
	~OBB3() {}

public:
	Vector3 center;
	Vector3 right;
	Vector3 up;
	Vector3 forward;

	//
	Vector3 center;
	Vector3 half_extents;
	//Quaternion orient;

// 	bool IsContained(const Vector3& pos) {
// 		Vector3 localPos = ((pos - center) * orient.GetInverse());
// 		return Abs(localPos) < half_extents;
// 
//		Vector3 localPos = pos * Inverse_space;
// 		return (Abs(pos - center) < half_extents));
// 	}
};
