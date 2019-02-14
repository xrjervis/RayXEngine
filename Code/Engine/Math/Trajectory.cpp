#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Trajectory.hpp"


Trajectory::Trajectory() {

}

Trajectory::~Trajectory() {
}

Vector2 Trajectory::Evaluate(float gravity, Vector2 launchVelocity, float time) {
	float x = launchVelocity.x * time;
	float y = -0.5f * gravity * time * time + launchVelocity.y * time;
	return Vector2(x, y);
}

Vector2 Trajectory::Evaluate(float gravity, float launchSpeed, float launchAngle, float time) {
	float x = launchSpeed * CosDegrees(launchAngle) * time;
	float y = -0.5f * gravity * time * time + launchSpeed * SinDegrees(launchAngle) * time;
	return Vector2(x, y);
}

float Trajectory::GetMinimumLaunchSpeed(float gravity, float distance) {
	float v = sqrt(gravity * distance);
	return v;
}

bool Trajectory::GetLaunchAngles(std::vector<float>& out, float gravity, /* gravity */ float launchSpeed, /* launch speed */ float distance, /* horizontal displacement desired */ float height /*= 0.0f*/) {
	out.clear();
	float a = (gravity * distance * distance) / (2.f * launchSpeed * launchSpeed);
	float b = -distance;
	float c = height + a;
	std::vector<float> tanTheta;
	QuadraticFormula(a, b, c, tanTheta);
	if (tanTheta.empty()) {
		return false;
	}
	else {
		for (auto val : tanTheta) {
			float theta = AtanDegrees(val);
			out.push_back(theta);
		}
	}
	return true;
}

Vector2 Trajectory::GetLaunchVelocity(float gravity, float apexHeight, /* must be greater than height */ float distance, /* target distance */ float height) {
	float vy = sqrt(2.f * gravity * apexHeight);
	float vx = 0.f;

	float a = height;
	float b = -distance * sqrt(2 * gravity * apexHeight);
	float c = 0.5f * gravity * distance * distance;
	std::vector<float> results;
	QuadraticFormula(a, b, c, results);
	if (results.size() == 2) {
		vx = results[1];
	}
	else if(results.size() == 1){
		vx = results[0];
	}
	else {
		return Vector2::ZERO;
	}

	return Vector2(vx, vy);
}

