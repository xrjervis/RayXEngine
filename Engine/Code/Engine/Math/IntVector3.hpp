#pragma once

class IntVector3 {
public:
	~IntVector3() = default;
	IntVector3() = default;
	explicit IntVector3(int x, int y, int z);

	const IntVector3 operator+(const IntVector3& intVecToAdd) const;
	const IntVector3 operator-(const IntVector3& intVecToAdd) const;
	bool operator==(const IntVector3& compare) const;
	bool operator!=(const IntVector3& compare) const;
	void operator+=(const IntVector3& intVectorToAdd);
	void operator-=(const IntVector3& intVectorToAdd);

public:
	int x = 0;
	int y = 0;
	int z = 0;
};

const IntVector3 Interpolate(const IntVector3& start, const IntVector3& end, float fractionTowardEnd);