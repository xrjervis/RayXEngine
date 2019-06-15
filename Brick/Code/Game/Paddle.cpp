#include "Game/Paddle.hpp"
#include "Game/TheApp.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/InputSystem/InputSystem.hpp"

Paddle::Paddle() {
	m_halfSize = Vector2(50.f, 15.f);
	m_position = Vector2(g_mainOutput->GetWidth() * 0.5f, 3.f * m_halfSize.y);
}

Paddle::~Paddle() {

}

void Paddle::Update(float ds) {
	Entity2d::Update(ds);

	m_velocity.x = 0.f;
	m_velocity.x = 0.f;
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_A)) {
		m_velocity.x = -200.f;
	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_D)) {
		m_velocity.x = 200.f;
	}

	
	m_position.x = ClampFloat(m_position.x, m_halfSize.x, g_mainOutput->GetWidth() - m_halfSize.x);
}

void Paddle::Render() const {
	g_theRHI->GetImmediateRenderer()->DrawQuad2D(m_position, Vector2(0.5f, 0.5f), 2.f * m_halfSize, Rgba::BLUE);
}

