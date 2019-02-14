#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

FloatRange::FloatRange(float initialMin, float initialMax)
	: min(initialMin)
	, max(initialMax) {
	GUARANTEE_OR_DIE(max >= min, "Max is smaller than Min");
}

FloatRange::FloatRange(float initialMinMax)
	: min(initialMinMax)
	, max(initialMinMax) {
	GUARANTEE_OR_DIE(max >= min, "Max is smaller than Min");
}

float FloatRange::GetRandomInRange() const {
	return GetRandomFloatInRange(min, max);
}

void FloatRange::SetFromText(const char* text) {
	char* end;
	this->min = std::strtof(text, &end);
	text = end;
	if(text[0] != '\0'){
		while (text[0] == ' ' || text[0] == '~') text++;
		this->max = std::strtof(text, &end);
	}
	else{
		this->max = this->min;
	}
}

bool DoRangesOverlap(const FloatRange& a, const FloatRange& b) {
	if (a.min <= b.max && b.min <= a.max) {
		return true;
	}
	else {
		return false;
	}
}

const FloatRange Interpolate(const FloatRange& start, const FloatRange& end, float fractionTowardEnd) {
	return FloatRange(
		Interpolate(start.min, end.min, fractionTowardEnd), 
		Interpolate(start.max, end.max, fractionTowardEnd)
	);
}
