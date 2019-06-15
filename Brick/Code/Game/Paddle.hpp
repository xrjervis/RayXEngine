#pragma once
#include "Game/Entity2d.hpp"

class Paddle : public Entity2d {
public:
	Paddle();
	~Paddle();

	void	Update(float ds) ;
	void	Render() const ;

};