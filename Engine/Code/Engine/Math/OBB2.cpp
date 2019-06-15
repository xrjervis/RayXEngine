#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/MathUtils.hpp"

OBB2::OBB2(const Vector2& center, const Vector2& baseX, const Vector2& baseY, const Vector2& halfExtensions) 
	:OBB2(center, baseX, baseY, halfExtensions.x, halfExtensions.y){

}

OBB2::OBB2(const Vector2& center, const Vector2& baseX, const Vector2& baseY, float halfExtensionX, float halfExtensionY) 
	:m_center(center), m_baseX(baseX), m_baseY(baseY), m_halfExtensionX(halfExtensionX), m_halfExtensionY(halfExtensionY) {

}

OBB2::OBB2(const Vector2& center, const Vector2& halfExtensions) 
	:OBB2(center, Vector2(1, 0), Vector2(0, 1), halfExtensions) {

}

OBB2::OBB2(const Vector2& center, float halfExtensionX, float halfExtensionY) 
	:OBB2(center, Vector2(halfExtensionX, halfExtensionY)) {
}

OBB2::OBB2() 
	:OBB2(Vector2::ZERO, 1.f, 1.f) {

}

OBB2::~OBB2() {

}

float OBB2::GetOrient() const {
	return m_baseX.GetOrientationDegrees();
}

void OBB2::SetOrient(float degrees) {
	m_baseX = PolarToCartesian(1.f, degrees);
	m_baseY = Vector2(-m_baseX.y, m_baseX.x);
}
