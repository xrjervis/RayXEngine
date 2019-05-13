#pragma once
#include "Game/Entity.hpp"

class Player : public Entity {
public:
	Player(World* world);
	virtual ~Player() = default;

	virtual void Update(float ds) override;
	virtual void Render() const override;

public:
	static Vector3 s_eyePosition;
};