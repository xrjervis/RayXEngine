#include "Engine/Math/AABB2.hpp"
#include <vector>
#include <unordered_map>

AABB2::AABB2(const AABB2 & copy) {
	mins = copy.mins;
	maxs = copy.maxs;
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY) {
	mins.x = minX;
	mins.y = minY;
	maxs.x = maxX;
	maxs.y = maxY;
}

AABB2::AABB2(const Vector2 & mins, const Vector2 & maxs) {
	this->mins = mins;
	this->maxs = maxs;
}

AABB2::AABB2(const Vector2 & center, float radiusX, float radiusY) {
	mins = center - Vector2(radiusX, radiusY);
	maxs = center + Vector2(radiusX, radiusY);
}

void AABB2::StretchToIncludePoint(float x, float y) {
	if(!IsPointInside(x, y)){
		if (x < mins.x) {
			mins.x = x;
		}
		else if (x > maxs.x){
			maxs.x = x;
		}
		if(y < mins.y){
			mins.y = y;
		}
		else if(y > maxs.y){
			maxs.y = y;
		}
	}
}

void AABB2::StretchToIncludePoint(const Vector2 & point) {
	StretchToIncludePoint(point.x, point.y);
}

AABB2 AABB2::AddPaddingToSides(float xPaddingRadius, float yPaddingRadius) {
	mins.x -= xPaddingRadius;
	mins.y -= yPaddingRadius;
	maxs.x += xPaddingRadius;
	maxs.y += yPaddingRadius;
	return *this;
}

void AABB2::Translate(const Vector2 & translation) {
	mins += translation;
	maxs += translation;
}

void AABB2::Translate(float translationX, float translationY) {
	mins.x += translationX;
	mins.y += translationY;
	maxs.x += translationX;
	maxs.y += translationY;
}

bool AABB2::IsPointInside(float x, float y) const {
	if(mins.x <= x && x <= maxs.x && mins.y <= y && y <= maxs.y){
		return true;
	}
	else{
		return false;
	}
}

bool AABB2::IsPointInside(const Vector2 & point) const {
	return IsPointInside(point.x, point.y);
}

Vector2 AABB2::GetDimensions() const {
	return Vector2(maxs.x - mins.x, maxs.y - mins.y);
}

Vector2 AABB2::GetCenter() const {
	return Vector2(
		(maxs.x + mins.x) / 2.f,
		(maxs.y + mins.y) / 2.f);
}

void AABB2::operator+=(const Vector2 & translation) {
	mins += translation;
	maxs += translation;
}

void AABB2::operator-=(const Vector2 & antiTranslation) {
	mins -= antiTranslation;
	maxs -= antiTranslation;
}

bool AABB2::operator==(const AABB2& other) const {
	return (mins == other.mins) && (maxs == other.maxs);
}

AABB2 AABB2::operator+(const Vector2 & translation) const {
	return AABB2(mins + translation, maxs + translation);
}

AABB2 AABB2::operator-(const Vector2 & antiTranslation) const {
	return AABB2(mins - antiTranslation, maxs - antiTranslation);
}

void AABB2::SetFromText(const char* text) {
	std::vector<float> elements;

	char* end = nullptr;
	while (text[0] != '\0') {
		while (text[0] == ' ' || text[0] == ',') text++;
		if (text[0] != '\0') {
			float element = std::strtof(text, &end);
			elements.push_back(element);
			text = end;
		}
	}

	mins.x = elements[0];
	mins.y = elements[1];
	maxs.x = elements[2];
	maxs.y = elements[3];
}


void operator>>(std::istringstream& iss, AABB2& toValue) {
	std::string text;
	iss >> text;
	toValue.SetFromText(text.c_str());
}

bool DoAABBsOverlap(const AABB2& a, const AABB2& b) {
    if(a.IsPointInside(b.mins) || a.IsPointInside(Vector2(b.mins.x, b.maxs.y)) ||
		a.IsPointInside(b.maxs) || a.IsPointInside(Vector2(b.maxs.x, b.mins.y))){
		return true;
	}
	else if(b.IsPointInside(a.mins) || b.IsPointInside(Vector2(a.mins.x, a.maxs.y)) ||
		b.IsPointInside(a.maxs) || b.IsPointInside(Vector2(a.maxs.x, a.mins.y))){
		return true;
	}
	else{
		return false;
	}
}

const AABB2 Interpolate(const AABB2& start, const AABB2& end, float fractionTowardEnd) {
	return AABB2(
		Interpolate(start.mins, end.mins, fractionTowardEnd),
		Interpolate(start.maxs, end.maxs, fractionTowardEnd)
	);
}