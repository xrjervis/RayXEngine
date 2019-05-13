#pragma once
#include "Engine/Core/Camera.hpp"
#include "Game/CameraMode.hpp"

class GameCamera : public Camera{
public:
	GameCamera() = default;
	virtual ~GameCamera() = default;

	eCameraMode m_cameraMode = CAMERA_MODE_OVERSHOULDER;
};