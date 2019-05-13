#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Game/Player.hpp"
#include "Game/TheApp.hpp"

Player::Player(World* world)
	: Entity(world) {
	//m_colliderType = COLLIDER_TYPE_UNIBALL;
	m_colliderType = COLLIDER_TYPE_AABB3;
}

void Player::Update(float ds) {
	Vector3 forwardInEngine = m_transform.GetForward();
	forwardInEngine.y = 0;
	forwardInEngine = forwardInEngine.GetNormalized();
	Vector3 forward(forwardInEngine.z, -forwardInEngine.x, forwardInEngine.y);

	Vector3 rightInEngine = m_transform.GetRight();
	rightInEngine.y = 0;
	rightInEngine = rightInEngine.GetNormalized();
	Vector3 right(rightInEngine.z, -rightInEngine.x, rightInEngine.y);

	Vector3 up(0.f, 0.f, 1.f);

	float speedXY;
	if (m_isOnGround) {
		speedXY = 5.f;
	}
	else {
		speedXY = 2.f;
	}

	m_acceleration = Vector3::ZERO;

	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_W)) {
		m_acceleration += forward * speedXY;
	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_S)) {
		m_acceleration += -forward * speedXY;
	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_A)) {
		m_acceleration += -right * speedXY;
	}
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_D)) {
		m_acceleration += right * speedXY;
	}

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_SPACE)) {
		if (m_isOnGround) {
			m_velocity += Vector3(0.f, 0.f, 5.f);
			m_isOnGround = false;
		}
	}

	Vector2 mouseDelta = g_theInput->GetMouseDelta();
	float angularSpeed = 0.5f;
	if (mouseDelta.x != 0.f) {
		m_transform.Rotate(Vector3(0.f, 1.f, 0.f) * mouseDelta.x * angularSpeed);
	}
	if (mouseDelta.y != 0.f) {
		m_transform.Rotate(Vector3(1.f, 0.f, 0.f) * mouseDelta.y * angularSpeed);

	}

	if (g_theApp->m_isDebugMode) {
		// Print Player Position
		DebugString(0.f, Stringf("Player Position: (%.2f, %.2f, %.2f)", m_transform.GetWorldPosition().z, -m_transform.GetWorldPosition().x, m_transform.GetWorldPosition().y), Rgba::WHITE, Rgba::WHITE);
	}
}

void Player::Render() const {
	if (m_colliderType == COLLIDER_TYPE_AABB3) {
		g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
		Vector4 posW(m_transform.GetWorldPosition() + 0.5f * Vector3(0.f, 1.8f, 0.f), 1.f);
		g_theRHI->GetImmediateRenderer()->DrawAABB3Box((posW * Matrix44::EngineToGame).xyz(), Vector3(0.6f, 0.6f, 1.8f), Rgba::CYAN);
	}
	else if (m_colliderType == COLLIDER_TYPE_UNIBALL) {
		g_theRHI->GetImmediateRenderer()->BindMaterial(g_theResourceManager->GetMaterial("wireframe"));

		Vector4 posW(m_transform.GetWorldPosition(), 1.f);
		g_theRHI->GetImmediateRenderer()->DrawUVSphere((posW * Matrix44::EngineToGame).xyz(), .5f, 16, 16, Rgba::CYAN);
	}
}

Vector3 Player::s_eyePosition = Vector3(0.f, 1.65f, 0.f);