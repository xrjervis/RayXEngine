#pragma once

class FloatRange {
public:
	~FloatRange() = default;
	explicit FloatRange() = default;
	explicit FloatRange(float initialMin, float initialMax);
	explicit FloatRange(float initialMinMax);

	float GetRandomInRange() const;
	void SetFromText(const char* text);

public:
	float min = 0.f;
	float max = 0.f;
};

bool DoRangesOverlap(const FloatRange& a, const FloatRange& b);
const FloatRange Interpolate(const FloatRange& start, const FloatRange& end, float fractionTowardEnd);