#include "Engine/Core/OrbitCamera.hpp"
#include "Engine/Math/MathUtils.hpp"

OrbitCamera::OrbitCamera() {
	Camera();
}

OrbitCamera::~OrbitCamera() {

}

void OrbitCamera::SetTarget(const Vector3& newTarget) {
	m_target = newTarget;
}

void OrbitCamera::SetSphericalCoordinate(float rad, float rot, float azi) {
	m_radius = rad;
	m_rotation = rot;
	m_azimuth = azi;
	m_targetPos = PolarToCartesian(rad, rot, azi) + m_target;
}

