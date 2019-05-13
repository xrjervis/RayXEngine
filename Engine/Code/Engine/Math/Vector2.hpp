#pragma once
#include <cmath>
#include "Engine/Math/IntVector2.hpp"
#include <sstream>

//-----------------------------------------------------------------------------------------------
class Vector2{
public:
	// Construction/Destruction
	~Vector2() = default;											// destructor: do nothing (for speed)
	Vector2() = default;											// default constructor: do nothing (for speed)
	Vector2(const Vector2& copyFrom);								// copy constructor (from another vec2)
	explicit Vector2(float initialX, float initialY);				// explicit constructor (from x, y)
	explicit Vector2(int initialX, int initialY);
	explicit Vector2(const IntVector2& initial);

	// Operators
	const	Vector2 operator+(const Vector2& vecToAdd) const;			// vec2 + vec2
	const	Vector2 operator-(const Vector2& vecToSubtract) const;		// vec2 - vec2
	const	Vector2 operator*(float uniformScale) const;				// vec2 * float
	const	Vector2 operator/(float inverseScale) const;				// vec2 / float
	void	operator+=(const Vector2& vecToAdd);						// vec2 += vec2
	void	operator-=(const Vector2& vecToSubtract);					// vec2 -= vec2
	void	operator*=(const float uniformScale);						// vec2 *= float
	void	operator/=(const float uniformDivisor);						// vec2 /= float
	void	operator=(const Vector2& copyFrom);							// vec2 = vec2
	bool	operator==(const Vector2& compare) const;					// vec2 == vec2
	bool	operator!=(const Vector2& compare) const;					// vec2 != vec2

	// Additional Methods
	float			GetLength() const;
	float			GetLengthSquared() const;							//faster than GetLength() since it skips the sqrtf()
	float			NormalizeAndGetLength();							//set my new length to 1.0f; keep my direction
	Vector2			GetNormalized() const;								//return a new vector, which is a normalized copy of me
	float			GetOrientationDegrees() const;						//return 0 for east(5, 0), 90 for north(0, 8), etc.
	void			SetOrientationDegrees(float degrees);
	static Vector2	MakeDirectionAtDegrees(float degrees);	//create vector at angle
	void			SetFromText(const char* text);
	IntVector2		GetAsIntVector2() const;

	friend const Vector2 operator*(float uniformScale, const Vector2& vecToScale);	// float * vec2
	friend const Vector2 operator-(const Vector2& vecToReverse);	// -vec2

	friend void operator>>(std::istringstream& iss, Vector2& toValue);

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

	static Vector2 ONE;
	static Vector2 ZERO;
};

float	CrossProduct(const Vector2& a, const Vector2& b);
float	DotProduct(const Vector2& a, const Vector2&b);
float	GetDistance(const Vector2& a, const Vector2& b);
float	GetDistanceSquared(const Vector2& a, const Vector2& b);

const Vector2 GetProjectedVector(const Vector2& vectorToProject, const Vector2& projectOnto);

const Vector2 GetTransformedIntoBasis(const Vector2& originalVector,
	const Vector2& newBasisI, const Vector2& newBasisJ);

const Vector2 GetTransformedOutOfBasis(const Vector2& vectorInBasis,
	const Vector2& oldBasisI, const Vector2& oldBasisJ);

void DecomposeVectorIntoBasis(const Vector2& originalVector,
	const Vector2& newBasisI, const Vector2& newBasisJ,
	Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ);

const Vector2 Interpolate(const Vector2& start, const Vector2& end, float fractionTowardEnd);
const Vector2 Reflect(const Vector2& originalVector, const Vector2 normalVector);

bool DoLineSegmentsIntersect(const Vector2& p0, const Vector2& p1, const Vector2& q0, const Vector2& q1, Vector2& intersectPoint);
bool DoRayAndLineSegmentsIntersect(const Vector2& rayStart, const Vector2& rayDir, const Vector2& l0, const Vector2& l1, Vector2& intersectPoint);

const Vector2 MoveTowards(const Vector2& current, const Vector2& target, float maxDist);
