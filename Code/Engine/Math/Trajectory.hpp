#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector2.hpp"
#include <deque>
#include <memory>
#include <vector>

class Trajectory {
public:
	Trajectory();
	~Trajectory();

	static Vector2 Trajectory::Evaluate( float gravity, Vector2 launchVelocity, float time );
	static Vector2 Trajectory::Evaluate( float gravity, float launchSpeed, float launchAngle, float time );
	static float Trajectory::GetMinimumLaunchSpeed( float gravity, float distance );
	static bool Trajectory::GetLaunchAngles(std::vector<float>& out,
		float gravity,          // gravity 
		float launchSpeed,		// launch speed
		float distance,         // horizontal displacement desired
		float height = 0.0f);	// vertical displacement desired
	//static float Trajectory::GetMaxHeight( float gravity, float launchSpeed, float distance );
	static Vector2 Trajectory::GetLaunchVelocity(float gravity,
		float apexHeight,       // must be greater than height
		float distance,         // target distance
		float height);         // target height
};