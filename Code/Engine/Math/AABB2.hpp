#pragma once
#include "Engine/Math/Vector2.hpp"
#include <sstream>
#include <functional>

class AABB2{
public:
	~AABB2() = default;
	explicit AABB2() : mins(Vector2(0,0)), maxs(Vector2(1,1)) {}
	AABB2(const AABB2& copy);
	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(const Vector2& mins, const Vector2& maxs);
	explicit AABB2(const Vector2& center, float radiusX, float radiusY);

	void	StretchToIncludePoint(float x, float y); //stretch, not move
	void	StretchToIncludePoint(const Vector2& point);
	AABB2	AddPaddingToSides(float xPaddingRadius, float yPaddingRadius);
	void	Translate(const Vector2& translation); //move the box, similar to +=
	void	Translate(float translationX, float translationY);

	bool	IsPointInside(float x, float y) const; // is "x,y" within box's interior?
	bool	IsPointInside(const Vector2& point) const;
	Vector2 GetDimensions() const; // return a Vector2 of (width, height)
	Vector2 GetCenter() const; // return the center position of the box
	void	SetFromText(const char* text);


	void	operator+=(const Vector2& translation); // move(translate) the box
	void	operator-=(const Vector2& antiTranslation);
	AABB2	operator+(const Vector2& translation) const; //create a (temp) moved box
	AABB2	operator-(const Vector2& antiTranslation) const;
	bool	operator==(const AABB2& other) const;
	friend void operator>>(std::istringstream& iss, AABB2& toValue);

public:
	Vector2 mins;
	Vector2 maxs;
};

bool DoAABBsOverlap(const AABB2& a, const AABB2& b);
const AABB2 Interpolate(const AABB2& start, const AABB2& end, float fractionTowardEnd);

namespace std {
	// custom specialization of std::hash can be injected in namespace std
	//https://en.cppreference.com/w/cpp/utility/hash

	template <>
	struct std::hash<AABB2> {
		std::size_t operator()(const AABB2& key) const {
			// Quick and Simple Hash Code Combinations
			//https://stackoverflow.com/questions/1646807/quick-and-simple-hash-code-combinations/1646913#1646913
			std::size_t hash = 17;
			hash = hash * 31 + std::hash<float>{}(key.mins.x);
			hash = hash * 31 + std::hash<float>{}(key.mins.y);
			hash = hash * 31 + std::hash<float>{}(key.maxs.x);
			hash = hash * 31 + std::hash<float>{}(key.maxs.y);

			return hash;
		}
	};
}