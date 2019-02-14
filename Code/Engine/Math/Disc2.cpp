#include "Disc2.hpp"
#include "Engine/Math/MathUtils.hpp"

Disc2::Disc2(const Disc2 & copyFrom)
	: center(copyFrom.center), 
	radius(copyFrom.radius) {}

Disc2::Disc2(float initialX, float initialY, float initialRadius) 
	: center(Vector2(initialX, initialY)),
	radius(initialRadius) {}

Disc2::Disc2(const Vector2 & initialCenter, float initialRadius)
	: center(initialCenter),
	radius(initialRadius) {}

void Disc2::StretchToIncludePoint(float x, float y) {
	if(!IsPointInside(x, y)) {
		radius = GetDistance(center, Vector2(x, y));
	}
}

void Disc2::StretchToIncludePoint(const Vector2 & point) {
	if (!IsPointInside(point)) {
		radius = GetDistance(center, point);
	}
}

void Disc2::AddPaddingToRadius(float paddingRadius) {
	radius += paddingRadius;
}

void Disc2::Translate(const Vector2 & translation) {
	center += translation;
}

void Disc2::Translate(float translationX, float translationY) {
	center.x += translationX;
	center.y += translationY;
}

bool Disc2::IsPointInside(float x, float y) const {
	return GetDistance(center, Vector2(x, y)) < radius;
}

bool Disc2::IsPointInside(const Vector2 & point) const {
	return GetDistance(center, point) < radius;
}

void Disc2::operator+=(const Vector2 & translation) {
	center += translation;
}

void Disc2::operator-=(const Vector2 & antiTranslation) {
	center -= antiTranslation;
}

Disc2 Disc2::operator+(const Vector2 & translation) const {
	return Disc2(center + translation, radius);
}

Disc2 Disc2::operator-(const Vector2 & antiTranslation) const {
	return Disc2(center - antiTranslation, radius);
}

bool DoDiscsOverlap(const Disc2 & a, const Disc2 & b) {
	return GetDistance(a.center, b.center) < (a.radius + b.radius);
}

bool DoDiscsOverlap(const Vector2 & aCenter, float aRadius, const Vector2 & bCenter, float bRadius) {
	return GetDistance(aCenter, bCenter) < (aRadius + bRadius);
}

const Disc2 Interpolate(const Disc2& start, const Disc2& end, float fractionTowardEnd) {
	return Disc2(
		Interpolate(start.center, end.center, fractionTowardEnd),
		Interpolate(start.radius, end.radius, fractionTowardEnd)
	);
}
