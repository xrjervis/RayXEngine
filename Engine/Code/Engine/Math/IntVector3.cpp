#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/MathUtils.hpp"

IntVector3::IntVector3(int x, int y, int z) 
	:x(x)
	,y(y)
	,z(z) {}

const IntVector3 IntVector3::operator+(const IntVector3& intVecToAdd) const {
	return IntVector3(this->x + intVecToAdd.x, this->y + intVecToAdd.y, this->z + intVecToAdd.z);
}

const IntVector3 IntVector3::operator-(const IntVector3& intVecToAdd) const {
	return IntVector3(this->x - intVecToAdd.x, this->y - intVecToAdd.y, this->z - intVecToAdd.z);
}

void IntVector3::operator-=(const IntVector3& intVectorToAdd) {
	this->x -= intVectorToAdd.x;
	this->y -= intVectorToAdd.y;
	this->z -= intVectorToAdd.z;
}

void IntVector3::operator+=(const IntVector3& intVectorToAdd) {
	this->x += intVectorToAdd.x;
	this->y += intVectorToAdd.y;
	this->z += intVectorToAdd.z;
}

bool IntVector3::operator!=(const IntVector3& compare) const {
	return !(*this == compare);
}

bool IntVector3::operator==(const IntVector3& compare) const {
	return (x == compare.x && y == compare.y && z == compare.z);
}

const IntVector3 Interpolate(const IntVector3& start, const IntVector3& end, float fractionTowardEnd) {
	return IntVector3(
		Interpolate(start.x, end.x, fractionTowardEnd),
		Interpolate(start.y, end.y, fractionTowardEnd),
		Interpolate(start.z, end.z, fractionTowardEnd)
	);
}
