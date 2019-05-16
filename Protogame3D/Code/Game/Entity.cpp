#include "Game/Entity.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

Entity::Entity(float age, const Vector3& pos, const Rgba& color)
	: m_age(age)
	, m_position(pos)
	, m_color(color) {

}

Entity::~Entity() {

}

void Entity::Update(float ds) {
	m_age -= ds;
	if (m_age <= 0.f) {
		m_isDead = true;
	}
}

void Entity::Render() const {
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);
	g_theRHI->GetImmediateRenderer()->DrawCube(m_position, Vector3(0.5f, 0.5f, 0.5f), m_color);
}

