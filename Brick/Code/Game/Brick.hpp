#pragma once
#include "Game/Entity2d.hpp"
#include "Engine/Math/AABB2.hpp"

class Brick : public Entity2d {
public:
	Brick();
	virtual ~Brick();

	void	Update(float ds) ;
	void	Render() const ;

public:
	AABB2	m_physicsBody;
};