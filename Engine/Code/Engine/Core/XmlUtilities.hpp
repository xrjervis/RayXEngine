#pragma once
#include "ThirdParty/pugixml/pugixml.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include <string>
#include <vector>

int ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, int defaultValue);
char ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, char defaultValue);
bool ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, bool defaultValue);
float ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, float defaultValue);
Rgba ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const Rgba& defaultValue);
Vector2 ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const Vector2& defaultValue);
IntRange ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const IntRange& defaultValue);
FloatRange ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const FloatRange& defaultValue);
IntVector2 ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const IntVector2& defaultValue);
std::string ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const std::string& defaultValue);
std::string ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const char* defaultValue);

template<typename T>
void SetFromText(std::vector<T>& out, const char* text) {
	char* end = nullptr;
	while (text[0] != '\0') {
		while (text[0] == ' ' || text[0] == ',') text++;
		T element = static_cast<T>(std::strtof(text, &end));
		out.push_back(element);
		text = end;
	}
}

template<typename T>
void
ParseXmlAttributeVector(const pugi::xml_node& node, const char* attributeName, std::vector<T>& out) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		SetFromText(out, attribute.as_string());
	}
}
