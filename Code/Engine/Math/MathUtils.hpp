#pragma once
#include <cmath>
#include <vector>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Core/type.hpp"

constexpr float PI = 3.141592653f;

float			Abs(float a);
int				Abs(int a);

float			ConvertRadiansToDegrees(const float& radians);
float			ConvertDegreesToRadians(const float& degrees);
float			CosDegrees(const float& degrees);
float			SinDegrees(const float& degrees);
float			TanDegrees(const float& degrees);
float			Atan2Degrees(float y, float x);
float			AtanDegrees(float value);
float			AsinDegrees(float value);
float			AcosDegrees(float value);
Vector2			PolarToCartesian(const float& radius, const float& degrees);

// rotation[0,180] azimuth[0,360]
Vector3			PolarToCartesian(const float& rad, const float& rot, const float& azi);	

// Random number generation
float			GetRandomFloatInRange(float rangeMin, float rangeMax);		// Gives floats in [min, max]
float			GetRandomFloatZeroToOne();									// Gives floats in [0.0f, 1.0f]
int				GetRandomIntInRange(int minInclusive, int maxInclusive);	// Gives integers in [min,max]
int				GetRandomIntLessThan(int maxNotInclusive);					// Gives integers in [0, max-1]
float			GetRandomFloatAorB(float A, float B);																		    
bool			CheckRandomChance(float chanceForSuccess);					// If 0.27f passed, returns true 27% of the time


// Rounding, clamping and range-mapping
// Get fraction part of a float
float			Fract(float value);
int				RoundToNearestInt(float inValue);															// 0.5 rounds up to 1; -0.5 rounds up to 0
uint			ClampUint(uint inValue, uint minInclusive, uint maxInclusive);
int				ClampInt(int inValue, int minInclusive, int maxInclusive);
float			ClampFloat(float inValue, float minInclusive, float maxInclusive);
float			ClampFloatZeroToOne(float inValue);
float			ClampFloatNegativeOneToOne(float inValue);
float			Clamp01(float inValue);

float			GetFractionInRange(float inValue, float rangeStart, float rangeEnd);						// Finds the % (as a fraction) of inValue in [rangeStart, rangeEnd]
float			Interpolate(float start, float end, float fractionTowardEnd);								// Finds the value at a certain % (fraction) in [rangeStart, rangeEnd]
int				Interpolate(int start, int end, float fractionTowardEnd);
unsigned char	Interpolate(unsigned char start, unsigned char end, float fractionTowardEnd);
float			MoveTowards(float current, float target, float maxDist);

float			RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);		// For a value in [inStart, inEnd], finds the corresponding value in [outStart, outEnd]
int				RangeMapInt(int inValue, int inStart, int inEnd, int outStart, int outEnd);

// Angle & Vector utilities
float			RoundAngle(float angle);
float			GetAngularDisplacement(float startDegrees, float endDegrees);					// Finds the "angular displacement" (or signed angular distance) from startDegrees to endDegrees in the smartest way
float			TurnToward(float currentDegrees, float goalDegrees, float maxTurnDegrees);		// I face currentDegrees and want to turn "toward" goalDegrees, by up to maxTurnDegrees



//Bitflag utilities
bool			AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck);
bool			AreBitsSet(unsigned short bitFlags16, unsigned short flagsToCheck);
bool			AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck);
void			SetBits(unsigned char& bitFlag8, unsigned char flagsToSet);
void			SetBits(unsigned int& bitFlags32, unsigned int flagsToSet);
void			ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear);
void			ClearBits(unsigned int& bitFlags32, unsigned int flagToClear);


//Easing functions
float	SmoothStart2(float t); // 2nd-degree smooth start (a.k.a. quadratic ease in?
float	SmoothStart3(float t); // 3rd-degree smooth start (a.k.a. cubic ease in?
float	SmoothStart4(float t); // 4th-degree smooth start (a.k.a. quartic ease in?
float	SmoothStop2(float t); // 2nd-degree smooth start (a.k.a. quadratic ease out?
float	SmoothStop3(float t); // 3rd-degree smooth start (a.k.a. cubic ease out?
float	SmoothStop4(float t); // 4th-degree smooth start (a.k.a. quartic ease out?
float	SmoothStep3(float t); // 3rd-degree smooth start/stop (a.k.a. smoothstep?

bool	QuadraticFormula(float a, float b, float c, std::vector<float>& results);

// Normal Mapping Utils
Vector3 CalculateTangent(const Vector3& deltaPos1, const Vector3& deltaPos2, const Vector2& deltaUV1, const Vector2& deltaUV2);
Vector3 CalculateBiTangent(const Vector3& deltaPos1, const Vector3& deltaPos2, const Vector2& deltaUV1, const Vector2& deltaUV2);