#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/StringUtils.hpp"

int ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, int defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if(attribute != nullptr){
		return attribute.as_int();
	}
	else{
		return defaultValue;
	}
}

char ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, char defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		return attribute.value()[0];
	}
	else {
		return defaultValue;
	}
}

bool ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, bool defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		return attribute.as_bool();
	}
	else {
		return defaultValue;
	}
}

float ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, float defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		return attribute.as_float();
	}
	else {
		return defaultValue;
	}
}

Rgba ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const Rgba& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		Rgba rgba;
		rgba.SetFromText(attribute.value());
		return rgba;
	}
	else {
		return defaultValue;
	}
}

Vector2 ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const Vector2& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		Vector2 vec2;
		vec2.SetFromText(attribute.value());
		return vec2;
	}
	else {
		return defaultValue;
	}
}

IntRange ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const IntRange& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		IntRange intRange;
		intRange.SetFromText(attribute.value());
		return intRange;
	}
	else {
		return defaultValue;
	}
}

FloatRange ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const FloatRange& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		FloatRange floatRange;
		floatRange.SetFromText(attribute.value());
		return floatRange;
	}
	else {
		return defaultValue;
	}
}

IntVector2 ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const IntVector2& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		IntVector2 intVec2;
		intVec2.SetFromText(attribute.value());
		return intVec2;
	}
	else {
		return defaultValue;
	}
}

std::string ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const std::string& defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		return attribute.value();
	}
	else {
		return defaultValue;
	}
}

std::string ParseXmlAttribute(const pugi::xml_node& node, const char* attributeName, const char* defaultValue) {
	pugi::xml_attribute attribute = node.attribute(attributeName);
	if (attribute != nullptr) {
		return attribute.value();
	}
	else {
		return Stringf("%s", defaultValue);
	}
}