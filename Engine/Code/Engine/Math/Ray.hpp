#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Plane.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Sphere.hpp"

class Ray {
public:
	Ray(const Vector3& pos, const Vector3& dir);
	~Ray() = default;

	Vector3 Evaluate(int t);
	void SetStepLength(float stepLength);

public:
	Vector3 m_origin;
	Vector3 m_direction;
	Vector3 m_currentPosition;
	float m_stepLength = 1.f;
};

bool DoIntersect(const Ray& ray, const Plane& plane, Vector3& out_point);
//bool DoIntersect(const Ray& ray, const AABB3& aabb3, Vector3& out_point);
bool DoIntersect(const Ray& ray, const Sphere& sphere, Vector3& out_point);