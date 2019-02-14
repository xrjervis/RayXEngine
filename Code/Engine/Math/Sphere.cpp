#include "Engine/Math/Sphere.hpp"

Sphere::Sphere(const Vector3& center, float radius) 
	: m_center(center)
	, m_radius(radius) {}

bool Sphere::IsPointInside(const Vector3& point) const {
	return (point - m_center).GetLengthSquared() <= (m_radius * m_radius);
}