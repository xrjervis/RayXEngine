#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>

IntVector2::IntVector2(int x, int y) :
	x(x),
	y(y) {}

const IntVector2 IntVector2::operator+(const IntVector2 & intVecToAdd) const {
	return IntVector2(this->x + intVecToAdd.x, this->y + intVecToAdd.y);
}

const IntVector2 IntVector2::operator-(const IntVector2& intVecToAdd) const {
	return IntVector2(this->x - intVecToAdd.x, this->y - intVecToAdd.y);
}

void IntVector2::operator-=(const IntVector2& intVectorToAdd) {
	this->x -= intVectorToAdd.x;
	this->y -= intVectorToAdd.y;
}

bool IntVector2::operator<(const IntVector2& intVectorToCompare) const{
// 		1.	if left.y < right.y return true
// 		2.	else if right.y < left.y return false
// 		3.	else if left.x < right.x return true
// 		4.	// else if right.x < left.x return false // technically don’t need this last one…
// 		5.	else return false // in this case they're actually equal, but either way "not less than"
	if (this->y < intVectorToCompare.y) {
		return true;
	}
	else if (intVectorToCompare.y < this->y) {
		return false;
	}
	else if (this->x < intVectorToCompare.x) {
		return true;
	}
	else {
		return false;
	}
}

void IntVector2::SetFromText(const char* text) {
	char* end;
	this->x = std::strtol(text, &end, 10);
	text = end;
	while (text[0] == ' ' || text[0] == ',') text++;
	this->y = std::strtol(text, &end, 10);
}

void operator>>(std::istringstream& iss, IntVector2& toValue) {
	std::string text;
	iss >> text;
	toValue.SetFromText(text.c_str());
}

void IntVector2::operator+=(const IntVector2& intVectorToAdd) {
	this->x += intVectorToAdd.x;
	this->y += intVectorToAdd.y;
}

bool IntVector2::operator==(const IntVector2& compare) const {
	return	(x == compare.x && y == compare.y);
}

bool IntVector2::operator!=(const IntVector2& compare) const {
	return !(*this == compare);
}

const IntVector2 Interpolate(const IntVector2& start, const IntVector2& end, float fractionTowardEnd) {
	return IntVector2(
		Interpolate(start.x, end.x, fractionTowardEnd),
		Interpolate(start.y, end.y, fractionTowardEnd)
	);
}
