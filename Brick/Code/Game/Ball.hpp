#pragma once
#include "Game/Entity2d.hpp"

class Ball : public Entity2d {
public:
	Ball();
	virtual ~Ball();

	void		Update(float ds);
	void		Render() const;

public:
	float		m_orientDegrees = 0.f;
};