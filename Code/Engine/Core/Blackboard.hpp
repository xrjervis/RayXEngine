#pragma once
#include <map>
#include <string>
#include "ThirdParty/pugixml/pugixml.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntRange.hpp"


class Blackboard{
public:
	~Blackboard() = default;
	Blackboard() {};

	void			PopulateFromXmlElementAttributes(const pugi::xml_node node);
	void			SetValue(const std::string& keyName, const std::string& newValue);

	bool			GetValue(const std::string& keyName, bool defaultValue) const;
	int				GetValue(const std::string& keyName, int defaultValue) const;
	float			GetValue(const std::string& keyName, float defaultValue) const;
	std::string		GetValue(const std::string& keyName, std::string defaultValue) const;
	std::string		GetValue(const std::string& keyName, const char* defaultValue) const;
	Rgba			GetValue(const std::string& keyName, const Rgba& defaultValue) const;
	Vector2			GetValue(const std::string& keyName, const Vector2& defaultValue) const;
	IntVector2		GetValue(const std::string& keyName, const IntVector2& defaultValue) const;
	FloatRange		GetValue(const std::string& keyName, const FloatRange& defaultValue) const;
	IntRange		GetValue(const std::string& keyName, const IntRange& defaultValue) const;

private:
	std::map<std::string, std::string> m_keyValuePairs;
};