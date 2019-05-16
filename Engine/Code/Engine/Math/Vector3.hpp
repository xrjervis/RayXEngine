#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector3.hpp"

class Vector3{
public:
	~Vector3() = default;											// destructor: do nothing (for speed)
	Vector3() = default;											// default constructor: do nothing (for speed)
	Vector3(const Vector3& copyFrom);								// copy constructor (from another vec3)
	Vector3(const Vector2& vec2);
	Vector3(const Vector2& vec2, float initialZ);
	Vector3(const IntVector3& intVector3);



	explicit Vector3(float initialX, float initialY, float initialZ);				// explicit constructor (from x, y)
	explicit Vector3(int initialX, int initialY, int initialZ);

	const Vector3 operator+(const Vector3& vecToAdd) const;
	const Vector3 operator-(const Vector3& vecToSubtract) const;
	const Vector3 operator*(float uniformScale) const;
	const Vector3 operator*(const Vector3& vecToMultiply) const;
	const Vector3 operator/(float inverseScale) const;
	void operator+=(const Vector3& vecToAdd);
	void operator-=(const Vector3& vecToSubtract);
	void operator*=(const float uniformScale);
	void operator*=(const Vector3& vecToMultiply);
	void operator/=(const float uniformDivisor);
	void operator=(const Vector3& copyFrom);
	bool operator==(const Vector3& compare) const;
	bool operator!=(const Vector3& compare) const;

	Vector2 xy() const;
	Vector2 xz() const;
	Vector2 yz() const;

	friend const Vector3 operator*(float uniformScale, const Vector3& vecToScale);	// float * vec3
	friend const Vector3 operator-(const Vector3& vecToReverse);	// -vec3

	float	GetLength() const;
	float	GetLengthSquared() const;
	Vector3	GetNormalized() const;

	void	SetFromText(const char* text);


public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	static Vector3 Up;
	static Vector3 Right;
	static Vector3 Forward;
	static Vector3 ZERO;
	static Vector3 ONE;

};

const Vector3 Interpolate(const Vector3& start, const Vector3& end, float fractionTowardEnd);
const Vector3 MoveTowards(const Vector3& current, const Vector3& target, float maxDist);
const Vector3 RotateTowards(const Vector3& current, const Vector3& target, float maxDegree);
const Vector3 Slerp(const Vector3& start, const Vector3& end, float t);
const Vector3 SlerpUnit(const Vector3& a, const Vector3& b, float t);

Vector3 CrossProduct(const Vector3& a, const Vector3& b);
float DotProduct(const Vector3& a, const Vector3& b);
float GetDistanceSquared(const Vector3& a, const Vector3& b);
float GetDistance(const Vector3& a, const Vector3& b);