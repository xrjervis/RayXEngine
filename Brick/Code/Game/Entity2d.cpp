#include "Game/Entity2d.hpp"


Entity2d::Entity2d() {

}

Entity2d::~Entity2d() {

}

void Entity2d::Update(float ds) {
	if (m_isAlive) {
		m_age += ds;
		m_velocity += m_acceleration * ds;
		m_position += m_velocity * ds;
	}
}

