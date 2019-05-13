#pragma once
#include "Engine/Math/Transform.hpp"
#include "Game/PhysicsMode.hpp"
#include "Game/ColliderType.hpp"

class World;

class Entity {
public:
	Entity(World* world) : m_world(world) {}
	virtual ~Entity() = default;

	virtual void Update(float ds) = 0;
	virtual void Render() const = 0;

public:
	World*			m_world = nullptr;

	Transform		m_transform;
	ePhysicsMode	m_physicsMode = PHYSICS_MODE_WALKING;
	eColliderType	m_colliderType = COLLIDER_TYPE_UNIBALL;
	Vector3			m_velocity;
	Vector3			m_acceleration;
	bool			m_isOnGround = false;
};