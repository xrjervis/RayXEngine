#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <cstdlib>

Vector2 Vector2::ONE = Vector2(1.f, 1.f);
Vector2 Vector2::ZERO = Vector2(0.f, 0.f);

//-----------------------------------------------------------------------------------------------
Vector2::Vector2(const Vector2& copy)
	: x(copy.x)
	, y(copy.y) {}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2(float initialX, float initialY)
	: x(initialX)
	, y(initialY) {}

Vector2::Vector2(int initialX, int initialY) {
	x = static_cast<float>(initialX);
	y = static_cast<float>(initialY);
}


Vector2::Vector2(const IntVector2& initial) {
	x = static_cast<float>(initial.x);
	y = static_cast<float>(initial.y);
}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator+(const Vector2& vecToAdd) const {
	return Vector2(this->x + vecToAdd.x, this->y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-(const Vector2& vecToSubtract) const {
	return Vector2(this->x - vecToSubtract.x, this->y - vecToSubtract.y);
}


//Member Function
const Vector2 Vector2::operator*(float uniformScale) const {
	return Vector2(this->x * uniformScale, this->y * uniformScale);
}

void operator>>(std::istringstream& iss, Vector2& toValue) {
	std::string text;
	iss >> text;
	toValue.SetFromText(text.c_str());
}

//Non-member Function
const Vector2 operator*(float uniformScale, const Vector2& vecToScale) {
	return Vector2(uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}

const Vector2 operator-(const Vector2& vecToReverse) {
	return Vector2(-vecToReverse.x, -vecToReverse.y);
}
//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/(float inverseScale) const {
	return Vector2(this->x * (1 / inverseScale), this->y * (1 / inverseScale));
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=(const Vector2& vecToAdd) {
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=(const Vector2& vecToSubtract) {
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=(const float uniformScale) {
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=(const float uniformDivisor) {
	x *= 1 / uniformDivisor;
	y *= 1 / uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator=(const Vector2& copyFrom) {
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
bool Vector2::operator==(const Vector2& compare) const {
	if(ApproxEqual(this->x, compare.x, 0.0001f) && ApproxEqual(this->y, compare.y, 0.0001f)){
		return true;
	}
	else{
		return false;
	}
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=(const Vector2& compare) const {
	if (this->x != compare.x || this->y != compare.y) {
		return true;
	}
	else {
		return false;
	}
}

float Vector2::GetLength() const {
	return sqrtf((x * x) + (y * y));
}

float Vector2::GetLengthSquared() const {
	return (x * x) + (y * y);
}

float Vector2::NormalizeAndGetLength() {
	float length = GetLength();
	if(length != 0){
		x *= 1.f / length;
		y *= 1.f / length;
	}
	return length;
}

Vector2 Vector2::GetNormalized() const {
	float length = GetLength();
	if(length != 0){
		return Vector2(x * (1.f / GetLength()), y * (1.f / GetLength()));
	}
	else{
		return Vector2(0, 0);
	}
}

float Vector2::GetOrientationDegrees() const {
	return Atan2Degrees(y, x);
}

void Vector2::SetOrientationDegrees(float degrees) {
	float length = GetLength();
	x = length * CosDegrees(degrees);
	y = length * SinDegrees(degrees);
}

Vector2 Vector2::MakeDirectionAtDegrees(float degrees) {
	return Vector2(CosDegrees(degrees), SinDegrees(degrees)).GetNormalized();
}

void Vector2::SetFromText(const char* text) {
	char* end;
	this->x = std::strtof(text, &end);
	text = end;
	while (text[0] == ' ' || text[0] == ',') text++;
	this->y = std::strtof(text, &end);
}

IntVector2 Vector2::GetAsIntVector2() const {
	return IntVector2((int)x, (int)y);
}

float CrossProduct(const Vector2& a, const Vector2& b) {
	return (a.x * b.y - a.y * b.x);
}

float DotProduct(const Vector2& a, const Vector2& b) {
	return (a.x * b.x) + (a.y * b.y);
}

float GetDistance(const Vector2 & a, const Vector2 & b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	return sqrtf((dx * dx) + (dy * dy));
}

float GetDistanceSquared(const Vector2 & a, const Vector2 & b) {
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	return (dx * dx) + (dy * dy);
}

const Vector2 GetProjectedVector(const Vector2& vectorToProject, const Vector2& projectOnto) {
	return DotProduct(vectorToProject, projectOnto.GetNormalized()) * projectOnto.GetNormalized();
}

const Vector2 GetTransformedIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ) {
	return Vector2(
		DotProduct(originalVector, newBasisI),
		DotProduct(originalVector, newBasisJ)
	);
}

const Vector2 GetTransformedOutOfBasis(const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ) {
	return (vectorInBasis.x * oldBasisI + vectorInBasis.y * oldBasisJ);
}

void DecomposeVectorIntoBasis(const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ) {
	out_vectorAlongI = GetProjectedVector(originalVector, newBasisI);
	out_vectorAlongJ = GetProjectedVector(originalVector, newBasisJ);
}

const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd) {
	return Vector2(Interpolate(start.x, end.x, fractionTowardEnd), Interpolate(start.y, end.y, fractionTowardEnd));
}

const Vector2 Reflect(const Vector2& originalVector, const Vector2 normalVector) {
	Vector2 vecOntoNormal = GetProjectedVector(originalVector, normalVector);
	return originalVector - 2 * vecOntoNormal;
}

bool DoLineSegmentsIntersect(const Vector2& p0, const Vector2& p1, const Vector2& q0, const Vector2& q1, Vector2& intersectPoint) {
	Vector2 line1 = p1 - p0;
	Vector2 line2 = q1 - q0;

	float t;
	float u;

	t = CrossProduct((q0 - p0), line2) / CrossProduct(line1, line2);
	u = CrossProduct((q0 - p0), line1) / CrossProduct(line1, line2);

	if (-0.0001f <= t && t <= 1.0001f && -0.0001f <= u && u <= 1.0001f) {
		intersectPoint = p0 + t * line1;
		return true;
	}
	return false;
}

bool DoRayAndLineSegmentsIntersect(const Vector2& rayStart, const Vector2& rayDir, const Vector2& l0, const Vector2& l1, Vector2& intersectPoint) {

	Vector2 line = l1 - l0;
	float t;
	t = CrossProduct((rayStart - l0), rayDir) / CrossProduct(line, rayDir);
	if (-0.0001f <= t && t <= 1.0001f) {
		intersectPoint = l0 + t * line;
		if (DotProduct(intersectPoint - rayStart, rayDir) > 0.f) {
			return true;
		}
	}
	return false;
}

const Vector2 MoveTowards(const Vector2& current, const Vector2& target, float maxDist) {
	Vector2 a = target - current;
	float magnitude = a.GetLength();
	if (magnitude <= maxDist || magnitude < 0.01f) {
		return target;
	}
	return current + a.GetNormalized() * maxDist;
}
