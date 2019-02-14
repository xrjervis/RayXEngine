#include "Engine/Math/MathUtils.hpp"

float Abs(float a) {
	if (a < 0.f) {
		return -a;
	}
	return a;
}

int Abs(int a) {
	if (a < 0) {
		return -a;
	}
	return a;
}

float ConvertRadiansToDegrees(const float& radians) {
	return radians * (180.0f / PI);
}

float ConvertDegreesToRadians(const float& degrees) {
	return degrees * (PI / 180.0f);
}

float CosDegrees(const float& degrees) {
	return cosf(ConvertDegreesToRadians(degrees));
}

float SinDegrees(const float& degrees) {
	return sinf(ConvertDegreesToRadians(degrees));
}

float TanDegrees(const float& degrees) {
	return tanf(ConvertDegreesToRadians(degrees));
}

float GetRandomFloatInRange(float rangeMin, float rangeMax) {
	return rangeMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
}

float GetRandomFloatZeroToOne() {
	return static_cast<float> (rand()) * (1.f / static_cast<float>(RAND_MAX));
}

int GetRandomIntInRange(int minInclusive, int maxInclusive) {
	return minInclusive + (rand() % (maxInclusive - minInclusive + 1));
}

int GetRandomIntLessThan(int maxNotInclusive) {
	return rand() % maxNotInclusive;
}

float GetRandomFloatAorB(float A, float B) {
	if (GetRandomFloatZeroToOne() > 0.5f) {
		return A;
	}
	else {
		return B;
	}
}

bool CheckRandomChance(float chanceForSuccess) {
	return GetRandomFloatZeroToOne() <= chanceForSuccess;
}

float Fract(float value) {
	return value - floorf(value);
}

float Atan2Degrees(float y, float x) {
	return ConvertRadiansToDegrees(atan2f(y, x));
}

float AtanDegrees(float value) {
	return ConvertRadiansToDegrees(atanf(value));
}

float AsinDegrees(float value) {
	return ConvertRadiansToDegrees(asinf(value));
}

float AcosDegrees(float value) {
	return ConvertRadiansToDegrees(acosf(value));
}

Vector2 PolarToCartesian(const float& radius, const float& degrees) {
	float x = radius * CosDegrees(degrees);
	float y = radius * SinDegrees(degrees);
	return Vector2(x, y);
}

Vector3 PolarToCartesian(const float& rad, const float& rot, const float& azi) {
	float x = rad * SinDegrees(rot) * CosDegrees(azi);
	float y = rad * CosDegrees(rot);
	float z = rad * SinDegrees(rot) * SinDegrees(azi);
	return Vector3(x, y, z);
}

float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd) {
	if (inStart == inEnd) {
		return (outStart + outEnd) * 0.5f;
	}
	float inRange = inEnd - inStart;
	float outRange = outEnd - outStart;
	float inRelativeToStart = inValue - inStart;
	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = fractionIntoRange * outRange;
	return outRelativeToStart + outStart;
}


int RangeMapInt(int inValue, int inStart, int inEnd, int outStart, int outEnd) {
	if (inStart == inEnd) {
		return (outStart + outEnd) / 2;
	}
	int inRange = inEnd - inStart;
	int outRange = outEnd - outStart;
	int inRelativeToStart = inValue - inStart;
	float fractionIntoRange = (float)inRelativeToStart / (float)inRange;
	int outRelativeToStart = (int)(fractionIntoRange * outRange);
	return outRelativeToStart + outStart;
}

float RoundAngle(float angle) {
	while (angle > 180.f) {
		angle -= 360.f;
	}
	while (angle < -180.f) {
		angle += 360.f;
	}
	return angle;
}

float GetAngularDisplacement(float startDegrees, float endDegrees) {
	float angularDisp = endDegrees - startDegrees;
	return RoundAngle(angularDisp);
}

float TurnToward(float currentDegrees, float goalDegrees, float maxTurnDegrees) {
	float angDisp = GetAngularDisplacement(currentDegrees, goalDegrees);
	if (angDisp > 0) {
		if (angDisp > maxTurnDegrees) {
			currentDegrees += maxTurnDegrees;
			currentDegrees = RoundAngle(currentDegrees);
		}
		else {
			currentDegrees = goalDegrees;
		}
	}
	else if (angDisp < 0) {
		if (-angDisp > maxTurnDegrees) {
			currentDegrees -= maxTurnDegrees;
			currentDegrees = RoundAngle(currentDegrees);
		}
		else {
			currentDegrees = goalDegrees;
		}
	}
	return currentDegrees;
}

bool AreBitsSet(unsigned char bitFlags8, unsigned char flagsToCheck) {
	return (bitFlags8 & flagsToCheck) == flagsToCheck;
}

bool AreBitsSet(unsigned short bitFlags16, unsigned short flagsToCheck) {
	return (bitFlags16 & flagsToCheck) == flagsToCheck;
}

bool AreBitsSet(unsigned int bitFlags32, unsigned int flagsToCheck) {
	return (bitFlags32 & flagsToCheck) == flagsToCheck;
}

void SetBits(unsigned char& bitFlag8, unsigned char flagsToSet) {
	bitFlag8 |= flagsToSet;
}

void SetBits(unsigned int& bitFlags32, unsigned int flagsToSet) {
	bitFlags32 |= flagsToSet;
}

void ClearBits(unsigned char& bitFlags8, unsigned char flagsToClear) {
	bitFlags8 &= ~flagsToClear;
}

void ClearBits(unsigned int & bitFlags32, unsigned int flagToClear) {
	bitFlags32 &= ~flagToClear;
}

float SmoothStart2(float t) {
	return t * t;
}

float SmoothStart3(float t) {
	return t * t * t;
}

float SmoothStart4(float t) {
	return t * t * t * t;
}

float SmoothStop2(float t) {
	return 1 - ((1 - t) * (1 - t));
}

float SmoothStop3(float t) {
	return 1 - ((1 - t) * (1 - t) * (1 - t));
}

float SmoothStop4(float t) {
	return 1 - ((1 - t) * (1 - t) * (1 - t) * (1 - t));
}

/*			    ----
		    ---
          --
        --
     ---
----
*/
float SmoothStep3(float t) {
	return (1 - t) * SmoothStart2(t) + t * SmoothStop2(t);
}

bool QuadraticFormula(float a, float b, float c, std::vector<float>& results) {
	results.clear();
	if (a == 0.f) {
		return false;
	}
	float delta = sqrt((b * b) - (4 * a * c));

	if (delta < 0) {
		return false;
	}
	else if (delta == 0) {
		float result = -b / (2 * a);
		results.push_back(result);
		return true;
	}
	else if (delta > 0) {
		float result_1 = (-b + delta) / (2 * a);
		float result_2 = (-b - delta) / (2 * a);
		results.push_back(result_1);
		results.push_back(result_2);
		return true;
	}
	return true;
}



int RoundToNearestInt(float inValue) {
	if (inValue > 0)
		return static_cast<int>(inValue + 0.5f);
	else
		return static_cast<int>(inValue - 0.49999f);
}

uint ClampUint(uint inValue, uint minInclusive, uint maxInclusive) {
	if (inValue > maxInclusive) {
		return maxInclusive;
	}
	else if (inValue < minInclusive) {
		return minInclusive;
	}
	else {
		return inValue;
	}
}

int ClampInt(int inValue, int minInclusive, int maxInclusive) {
	if (inValue > maxInclusive) {
		return maxInclusive;
	}
	else if (inValue < minInclusive) {
		return minInclusive;
	}
	else {
		return inValue;
	}
}

float ClampFloat(float inValue, float minInclusive, float maxInclusive) {
	if (inValue > maxInclusive) {
		return maxInclusive;
	}
	else if (inValue < minInclusive) {
		return minInclusive;
	}
	else {
		return inValue;
	}
}

float ClampFloatZeroToOne(float inValue) {
	return ClampFloat(inValue, 0.f, 1.f);
}

float ClampFloatNegativeOneToOne(float inValue) {
	return ClampFloat(inValue, -1.f, 1.f);
}

float Clamp01(float inValue) {
	return ClampFloat(inValue, 0.f, 1.f);
}

float GetFractionInRange(float inValue, float rangeStart, float rangeEnd) {
	return ClampFloatZeroToOne((inValue - rangeStart) / (rangeEnd - rangeStart));
}

float Interpolate(float start, float end, float fractionTowardEnd) {
	if (Abs(end - start) < 0.001f) {
		return end;
	}
	return ((end - start) * fractionTowardEnd) + start;
}

int Interpolate(int start, int end, float fractionTowardEnd) {
	return RoundToNearestInt(static_cast<float>(end - start) * fractionTowardEnd) + start;
}

unsigned char Interpolate(unsigned char start, unsigned char end, float fractionTowardEnd) {
	return static_cast<unsigned char>(Interpolate((float)start, (float)end, fractionTowardEnd));
}

float MoveTowards(float current, float target, float maxDist) {
	if (target >= current) {
		float distance = target - current;
		if (distance <= maxDist) {
			return target;
		}
		else {
			return current + maxDist;
		}
	}
	else {
		float distance = current - target;
		if (distance <= maxDist) {
			return target;
		}
		else {
			return current - maxDist;
		}
	}
}

Vector3 CalculateTangent(const Vector3& deltaPos1, const Vector3& deltaPos2, const Vector2& deltaUV1, const Vector2& deltaUV2) {
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	Vector3 tangent;
	tangent.x = f * (deltaUV2.y * deltaPos1.x - deltaUV1.y * deltaPos2.x);
	tangent.y = f * (deltaUV2.y * deltaPos1.y - deltaUV1.y * deltaPos2.y);
	tangent.z = f * (deltaUV2.y * deltaPos1.z - deltaUV1.y * deltaPos2.z);
	tangent = tangent.GetNormalized();

	return tangent;
}

Vector3 CalculateBiTangent(const Vector3& deltaPos1, const Vector3& deltaPos2, const Vector2& deltaUV1, const Vector2& deltaUV2) {
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	Vector3 bitangent;
	bitangent.x = f * (-deltaUV2.x * deltaPos1.x + deltaUV1.x * deltaPos2.x);
	bitangent.y = f * (-deltaUV2.x * deltaPos1.y + deltaUV1.x * deltaPos2.y);
	bitangent.z = f * (-deltaUV2.x * deltaPos1.z + deltaUV1.x * deltaPos2.z);
	bitangent = bitangent.GetNormalized();

	return bitangent;

}