#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include "Engine/Core/ErrorWarningAssert.hpp"


void Blackboard::PopulateFromXmlElementAttributes(const pugi::xml_node node) {
	pugi::xml_node_iterator nodeIt = node.begin();
	for (pugi::xml_attribute_iterator attrIt = nodeIt->attributes_begin(); attrIt != nodeIt->attributes_end(); ++attrIt) {
		m_keyValuePairs.insert({attrIt->name(), attrIt->as_string()});
	}
}

void Blackboard::SetValue(const std::string& keyName, const std::string& newValue) {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		iter->second = newValue;
	}
}

bool Blackboard::GetValue(const std::string& keyName, bool defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);

	if(iter != m_keyValuePairs.end()){
		if(iter->second == "true"){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return defaultValue;
	}
}

int Blackboard::GetValue(const std::string& keyName, int defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()){
		return std::stoi(iter->second);
	}
	else{
		return defaultValue;
	}
}

float Blackboard::GetValue(const std::string& keyName, float defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		return std::stof(iter->second);
	}
	else {
		return defaultValue;
	}
}

std::string Blackboard::GetValue(const std::string& keyName, std::string defaultValue = nullptr) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		return iter->second;
	}
	else {
		return defaultValue;
	}
}

std::string Blackboard::GetValue(const std::string& keyName, const char* defaultValue = nullptr) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		return iter->second;
	}
	else {
		return std::string(defaultValue);
	}
}

Rgba Blackboard::GetValue(const std::string& keyName, const Rgba& defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		Rgba rgba;
		rgba.SetFromText(iter->second.c_str());
		return rgba;
	}
	else {
		return defaultValue;
	}
}

Vector2 Blackboard::GetValue(const std::string& keyName, const Vector2& defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		Vector2 vector2;
		vector2.SetFromText(iter->second.c_str());
		return vector2;
	}
	else {
		return defaultValue;
	}
}

IntVector2 Blackboard::GetValue(const std::string& keyName, const IntVector2& defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		IntVector2 intVector2;
		intVector2.SetFromText(iter->second.c_str());
		return intVector2;
	}
	else {
		return defaultValue;
	}
}

FloatRange Blackboard::GetValue(const std::string& keyName, const FloatRange& defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		FloatRange floatRange;
		floatRange.SetFromText(iter->second.c_str());
		return floatRange;
	}
	else {
		return defaultValue;
	}
}

IntRange Blackboard::GetValue(const std::string& keyName, const IntRange& defaultValue) const {
	auto iter = m_keyValuePairs.find(keyName);
	if (iter != m_keyValuePairs.end()) {
		IntRange intRange;
		intRange.SetFromText(iter->second.c_str());
		return intRange;
	}
	else {
		return defaultValue;
	}
}