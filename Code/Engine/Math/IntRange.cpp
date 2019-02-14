#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

IntRange::IntRange(int initialMin, int initialMax)
	: min(initialMin)
	, max(initialMax) { 
	GUARANTEE_OR_DIE(max >= min, "Max is smaller than Min");
}

IntRange::IntRange(int initialMinMax)
	: min(initialMinMax)
	, max(initialMinMax) {
	GUARANTEE_OR_DIE(max >= min, "Max is smaller than Min");
}

int IntRange::GetRandomInRange() const {
	return GetRandomIntInRange(min, max);
}

void IntRange::SetFromText(const char* text) {
	char* end;
	this->min = std::strtol(text, &end, 10);
	text = end;
	if(text[0] != '\0'){
		while (text[0] == ' ' || text[0] == '~') text++;
		this->max = std::strtol(text, &end, 10);
	}
	else{
		this->max = this->min;
	}
}

bool DoRangesOverlap(const IntRange& a, const IntRange& b) {
	if (a.min <= b.max && b.min <= a.max) {
		return true;
	}
	else {
		return false;
	}
}

const IntRange Interpolate(const IntRange& start, const IntRange& end, float fractionTowardEnd) {
	return IntRange(
		Interpolate(start.min, end.min, fractionTowardEnd),
		Interpolate(start.max, end.max, fractionTowardEnd)
	);
}
