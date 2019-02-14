#include "Engine/Math/Ray.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

Ray::Ray(const Vector3& pos, const Vector3& dir) 
	: m_origin(pos)
	, m_direction(dir)
	, m_currentPosition(pos) {}

Vector3 Ray::Evaluate(int t) {
	m_currentPosition = m_origin + m_direction * (float)t * m_stepLength;
	return m_currentPosition;
}

void Ray::SetStepLength(float stepLength) {
	m_stepLength = stepLength;
}

bool DoIntersect(const Ray& ray, const Plane& plane, Vector3& out_point) {
	Vector3 n = plane.m_normal;
	Vector3 l = ray.m_direction;
	Vector3 p0 = plane.m_position;
	Vector3 l0 = ray.m_origin;

	float denom = DotProduct(n, l);
	if (denom > 0.001f) {
		Vector3 pl = p0 - l0;
		float t = DotProduct(pl, n) / denom;
		if (t >= 0) {
			out_point = p0 + t * l;
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

bool DoIntersect(const Ray& ray, const Sphere& sphere, Vector3& out_point) {
	Vector3 p0 = ray.m_origin;
	Vector3 d = ray.m_direction;
	Vector3 c = sphere.m_center;
	float r = sphere.m_radius;
	float r2 = r * r;

	Vector3 e = c - p0;
	float a = DotProduct(e, d);
	float e2 = DotProduct(e, e);
	float a2 = a * a;
	float b2 = e2 - a2;
	float f2 = r2 - b2;

	if (f2 < 0) {
		return false;
	}
	else {
		float f = sqrt(f2);
		float t = a - f;
		out_point = p0 + d * t;
		return true;
	}
}
