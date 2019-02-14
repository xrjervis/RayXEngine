#pragma once
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector3.hpp"
//
// struct RaycastResult2D {
// public:
// 	bool		m_didImpact = false;
// 	Vector2		m_impactPos = Vector2(0, 0);
// 	IntVector2	m_impactTileCoords = IntVector2(0, 0);
// 	float		m_impactDistance = 0.f;
// 	float		m_impactFraction = 1.f;
// 	Vector2		m_impactSurfaceNormal = Vector2(0, 0);
// };

struct RaycastResult_t {
public:
	bool		m_didImpact = false;
	Vector3		m_impactPosition = Vector3::ZERO;
	Vector3		m_impactNormal = Vector3(0, 1, 0);
	float		m_impactDistance = 0.f;
};