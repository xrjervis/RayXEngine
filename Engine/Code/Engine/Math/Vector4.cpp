#include "Engine/Math/Vector4.hpp"

Vector4 Vector4::ZERO = Vector4(0, 0, 0, 0);
Vector4 Vector4::ONE = Vector4(1, 1, 1, 1);

Vector4::Vector4(float initialX, float initialY, float initialZ, float initialW) 
	: x(initialX)
	, y(initialY)
	, z(initialZ) 
	, w(initialW){	

}

Vector4::Vector4(int initialX, int initialY, int initialZ, int initialW)
	: x(static_cast<float>(initialX))
	, y(static_cast<float>(initialY))
	, z(static_cast<float>(initialZ)) 
	, w(static_cast<float>(initialW)){

}

Vector4::Vector4(const Vector3& xyz, float initW) 
	: x(xyz.x)
	, y(xyz.y)
	, z(xyz.z)
	, w(initW){

}

Vector3 Vector4::xyz() const {
	return Vector3(x, y, z);
}

