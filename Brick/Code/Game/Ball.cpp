#include "Game/Ball.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

Ball::Ball() {
	m_orientDegrees = 0.f;
	m_halfSize = Vector2(5.f, 5.f);
	m_velocity = Vector2(0.5f, 0.2f) * 300.f;
}

Ball::~Ball() {

}

void Ball::Update(float ds) {
	Entity2d::Update(ds);
	
	// Check boundary
	if (m_position.x < m_halfSize.x || 
		m_position.x > g_mainOutput->GetWidth() - m_halfSize.x) {
		m_velocity -= 2.f * Vector2(m_velocity.x, 0.f);
	}
	if (m_position.y < m_halfSize.y ||
		m_position.y > g_mainOutput->GetHeight() - m_halfSize.y) {
		m_velocity -= 2.f * Vector2(0.f, m_velocity.y);
	}

	
}

void Ball::Render() const {
	Vector2 rightVector = PolarToCartesian(1.f, m_orientDegrees);
	Vector2 upVector(-rightVector.y, rightVector.x);

	g_theRHI->GetImmediateRenderer()->DrawOBB2D(m_position, rightVector, upVector, m_halfSize);
}

