#pragma once

class IntRange{
public:
	~IntRange() = default;
	explicit IntRange() = default;
	explicit IntRange(int initialMin, int initialMax);
	explicit IntRange(int initialMinMax);

	int GetRandomInRange() const;
	void SetFromText(const char* text);

public:
	int min = 0;
	int max = 0;
};

const IntRange Interpolate(const IntRange& start, const IntRange& end, float fractionTowardEnd);