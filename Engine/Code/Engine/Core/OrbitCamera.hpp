#pragma once
#include "Engine/Core/Camera.hpp"

class OrbitCamera : public Camera {
public:
	OrbitCamera();
	virtual ~OrbitCamera();

	void SetTarget(const Vector3& newTarget);
	void SetSphericalCoordinate(float rad, float rot, float azi);

public:
	Vector3 m_target;
	Vector3 m_currentTarget;
	Vector3 m_currentPos;
	Vector3 m_targetPos;
	float m_radius;     //distance from target
	float m_rotation;	//rotation around Y
	float m_azimuth;	//rotation toward up after previous rotation
};