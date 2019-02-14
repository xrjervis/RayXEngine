#pragma once
#include <sstream>
class IntVector2{
public:
	~IntVector2()= default;
	IntVector2() = default;
	explicit IntVector2(int x, int y);

	const IntVector2 operator+(const IntVector2& intVecToAdd) const;
	const IntVector2 operator-(const IntVector2& intVecToAdd) const;
	bool operator==(const IntVector2& compare) const;
	bool operator!=(const IntVector2& compare) const;
	void operator+=(const IntVector2& intVectorToAdd);
	void operator-=(const IntVector2& intVectorToAdd);
	bool operator<(const IntVector2& intVectorToCompare) const;
	void SetFromText(const char* text);

	friend void operator>>(std::istringstream& iss, IntVector2& toValue);

public:
	int x = 0;
	int y = 0;
};

const IntVector2 Interpolate(const IntVector2& start, const IntVector2& end, float fractionTowardEnd);