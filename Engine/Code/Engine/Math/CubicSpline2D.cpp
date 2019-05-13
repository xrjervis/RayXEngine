#include "Engine/Math/CubicSpline2D.hpp"
#include "Engine/Core/EngineCommon.hpp"

CubicSpline2D::CubicSpline2D(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray/*=nullptr */)	{
	AppendPoints(positionsArray, numPoints, velocitiesArray);
}

void CubicSpline2D::AppendPoint(const Vector2& position, const Vector2& velocity/*=Vector2::ZERO */) {
	m_positions.push_back(position);
	m_velocities.push_back(velocity);
}

void CubicSpline2D::AppendPoints(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray/*=nullptr */) {
	for(int i = 0; i < numPoints; ++i){
		if(velocitiesArray){
			AppendPoint(positionsArray[i], velocitiesArray[i]);
		}
		else{
			AppendPoint(positionsArray[i]);
		}
	}
}

void CubicSpline2D::InsertPoint(int insertBeforeIndex, const Vector2& position, const Vector2& velocity/*=Vector2::ZERO */) {
	auto it_pos = m_positions.begin();
	auto it_vel = m_velocities.begin();
	m_positions.insert(it_pos + insertBeforeIndex, position);
	m_velocities.insert(it_vel + insertBeforeIndex, velocity);
}

void CubicSpline2D::RemovePoint(int pointIndex) {
	auto it_pos = m_positions.begin();
	auto it_vel = m_velocities.begin();
	m_positions.erase(it_pos + pointIndex);
	m_velocities.erase(it_vel + pointIndex);
}

void CubicSpline2D::RemoveAllPoints() {
	m_positions.clear();
	m_velocities.clear();
}

void CubicSpline2D::SetPoint(int pointIndex, const Vector2& newPosition, const Vector2& newVelocity) {
	SetPosition(pointIndex, newPosition);
	SetVelocity(pointIndex, newVelocity);
}

void CubicSpline2D::SetPosition(int pointIndex, const Vector2& newPosition) {
	m_positions[pointIndex] = newPosition;
}

void CubicSpline2D::SetVelocity(int pointIndex, const Vector2& newVelocity) {
	m_velocities[pointIndex] = newVelocity;
}

void CubicSpline2D::SetCardinalVelocities(float tension/*=0.f*/, const Vector2& startVelocity/*=Vector2::ZERO*/, const Vector2& endVelocity/*=Vector2::ZERO */) {
	UNUSED(startVelocity);
	UNUSED(endVelocity);
	for(size_t i = 1; i < m_velocities.size() - 1; ++i){
		m_velocities[i] = ((1.f - tension) * (m_positions[i + 1] - m_positions[i - 1])) / 2.f;
	}
}

const Vector2 CubicSpline2D::GetPosition(int pointIndex) const{
	return m_positions[pointIndex];
}

const Vector2 CubicSpline2D::GetVelocity(int pointIndex) const{
	return m_velocities[pointIndex];
}

int CubicSpline2D::GetPositions(std::vector<Vector2>& out_positions) const{
	out_positions = m_positions;
	return (int)m_positions.size();
}

int CubicSpline2D::GetVelocities(std::vector<Vector2>& out_velocities) const{
	out_velocities = m_velocities;
	return (int)m_velocities.size();
}

Vector2 CubicSpline2D::EvaluateAtCumulativeParametric(float t) const {
	int t_integer = (int)t;
	float t_decimal = t - (float)t_integer;

	return EvaluateCubicHermite(m_positions[t_integer], m_velocities[t_integer], m_positions[t_integer + 1], m_velocities[t_integer + 1], t_decimal);
}

Vector2 CubicSpline2D::EvaluateAtNormalizedParametric(float t) const {
	int splineAmount = (int)m_positions.size() - 1;
	int currentSplineIndex;
	float currentInternal = 0.f;
	float internalLength = 1.f / (float)splineAmount;

	for(currentSplineIndex = 0; currentSplineIndex < splineAmount; ++currentSplineIndex){
		currentInternal = (float)(currentSplineIndex + 1) * internalLength;
		if(t < currentInternal){
			break;
		}
	}

	float t_internal = RangeMapFloat(t, currentInternal - internalLength, currentInternal, 0.f, 1.f);

	return EvaluateCubicHermite(m_positions[currentSplineIndex], m_velocities[currentSplineIndex], m_positions[currentSplineIndex + 1], m_velocities[currentSplineIndex + 1], t_internal);
}

