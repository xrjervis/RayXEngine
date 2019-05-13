#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/MathUtils.hpp"

Vector3 Vector3::Up(0.f, 1.f, 0.f);
Vector3 Vector3::Right(1.f, 0.f, 0.f);
Vector3 Vector3::Forward(0.f, 0.f, 1.f);
Vector3 Vector3::ZERO(0.f, 0.f, 0.f);
Vector3 Vector3::ONE(1.f, 1.f, 1.f);


Vector3::Vector3(const Vector3& copyFrom) 
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z){

}

Vector3::Vector3(float initialX, float initialY, float initialZ) 
	: x(initialX)
	, y(initialY)
	, z(initialZ) {
}

Vector3::Vector3(int initialX, int initialY, int initialZ) 
	: x(static_cast<float>(initialX))
	, y(static_cast<float>(initialY))
	, z(static_cast<float>(initialZ)){
}

Vector3::Vector3(const Vector2& vec2) 
	: x(vec2.x)
	, y(vec2.y)
	, z(0.f) {
}

Vector3::Vector3(const Vector2& vec2, float initialZ) 
	: x(vec2.x)
	, y(vec2.y)
	, z(initialZ) {
}

Vector3::Vector3(const IntVector3& intVector3) 
	: x((float)intVector3.x)
	, y((float)intVector3.y)
	, z((float)intVector3.z){
}

const Vector3 Vector3::operator+(const Vector3& vecToAdd) const {
	return Vector3(this->x + vecToAdd.x, this->y + vecToAdd.y, this->z + vecToAdd.z);
}

const Vector3 Vector3::operator-(const Vector3& vecToSubtract) const {
	return Vector3(this->x - vecToSubtract.x, this->y - vecToSubtract.y, this->z - vecToSubtract.z);
}

Vector2 Vector3::xy() const {
	return Vector2(x, y);
}

Vector2 Vector3::xz() const {
	return Vector2(x, z);
}

Vector2 Vector3::yz() const {
	return Vector2(y, z);
}

const Vector3 Vector3::operator*(float uniformScale) const {
	return Vector3(this->x * uniformScale, this->y * uniformScale, this->z * uniformScale);
}

const Vector3 Vector3::operator*(const Vector3& vecToMultiply) const {
	return Vector3(this->x * vecToMultiply.x, this->y * vecToMultiply.y, this->z * vecToMultiply.z);
}

const Vector3 Vector3::operator/(float inverseScale) const {
	return Vector3(this->x * (1 / inverseScale), this->y * (1 / inverseScale), this->z * (1 / inverseScale));
}

void Vector3::operator/=(const float uniformDivisor) {
	x *= 1 / uniformDivisor;
	y *= 1 / uniformDivisor;
	z *= 1 / uniformDivisor;
}

void Vector3::operator*=(const Vector3& vecToMultiply) {
	x *= vecToMultiply.x;
	y *= vecToMultiply.y;
	z *= vecToMultiply.z;
}

void Vector3::operator*=(const float uniformScale) {
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

void Vector3::operator-=(const Vector3& vecToSubtract) {
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}

void Vector3::operator+=(const Vector3& vecToAdd) {
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

void Vector3::operator=(const Vector3& copyFrom) {
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

bool Vector3::operator==(const Vector3& compare) const {
	if (this->x == compare.x && this->y == compare.y && this->z == compare.z) {
		return true;
	}
	else {
		return false;
	}
}

bool Vector3::operator!=(const Vector3& compare) const {
	if (this->x != compare.x || this->y != compare.y || this->z != compare.z) {
		return true;
	}
	else {
		return false;
	}
}

const Vector3 operator*(float uniformScale, const Vector3& vecToScale) {
	return Vector3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}

const Vector3 operator-(const Vector3& vecToReverse) {
	return Vector3(-vecToReverse.x, -vecToReverse.y, -vecToReverse.z);
}

float Vector3::GetLength() const {
	return sqrtf((x * x) + (y * y) + (z * z));
}

float Vector3::GetLengthSquared() const {
	return (x * x) + (y * y) + (z * z);
}

Vector3 Vector3::GetNormalized() const {
	float length = GetLength();
	if (length != 0) {
		return Vector3(x * (1.f / GetLength()), y * (1.f / GetLength()), z * (1.f / GetLength()));
	}
	else {
		return Vector3(0, 0, 0);
	}
}

const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd) {
	fractionTowardEnd = Clamp01(fractionTowardEnd);
	return (1.f - fractionTowardEnd) * start + fractionTowardEnd * end;
}

const Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDist) {
	Vector3 a = target - current;
	float magnitude = a.GetLength();
	if (magnitude <= maxDist || magnitude < 0.01f) {
		return target;
	}
	return current + a.GetNormalized() * maxDist;
}

const Vector3 RotateTowards(const Vector3& current, const Vector3& target, float maxDegree) {
	return Slerp(current, target, maxDegree / 360.f);
}

const Vector3 Slerp(const Vector3& start, const Vector3& end, float t) {
	float cosangle = ClampFloat(DotProduct(start.GetNormalized(), end.GetNormalized()), -1.0f, 1.0f);
	float angle = AcosDegrees(cosangle);
	t = Clamp01(t);
	float k0 = SinDegrees((1.f - t) * angle) / SinDegrees(angle);
	float k1 = SinDegrees(t * angle) / SinDegrees(angle);

	return k0 * start + k1 * end;
}

const Vector3 SlerpUnit(const Vector3& a, const Vector3& b, float t) {
	float cosangle = ClampFloat(DotProduct(a, b), -1.0f, 1.0f);
	float angle = AcosDegrees(cosangle);
	if (angle < 0.1f) {
		return Interpolate(a, b, t);
	}
	else {
		float pos_num = SinDegrees(t * angle);
		float neg_num = SinDegrees((1.0f - t) * angle);
		float den = SinDegrees(angle);

		return (neg_num / den) * a + (pos_num / den) * b;
	}
}

Vector3 CrossProduct(const Vector3& a, const Vector3& b) {
	Vector3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

float DotProduct(const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float GetDistanceSquared(const Vector3& a, const Vector3& b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;
	return (dx * dx) + (dy * dy) + (dz * dz);
}

float GetDistance(const Vector3& a, const Vector3& b) {
	return sqrt(GetDistanceSquared(a, b));
}
