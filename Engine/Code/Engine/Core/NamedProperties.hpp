#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <string>
#include <map>
#include <memory>

class PropertyBase {
public:
	PropertyBase() = default;
	virtual ~PropertyBase() = default;
	template<typename T>
	const T& Get() const {
		const Property<T>* ptr = dynamic_cast<const Property<T>*>(this);
		if (!ptr) {
			ERROR_AND_DIE("Named property type does not match!");
		}
		else {
			return ptr->Get();
		}
	}

	template<typename T>
	void Set(const T& rhs) {
		dynamic_cast<Property<T>*>(this)->Set(rhs);
	}
};

template <typename T>
class Property : public PropertyBase {
public:
	Property() = default;
	virtual ~Property() = default;
	Property(const T& rhs) : m_value(rhs) {}
	const T& Get() const { return m_value; }
	void Set(const T& rhs) { m_value = rhs; }

private:
	T m_value;
};

class NamedProperties {
public:
	NamedProperties() = default;
	~NamedProperties() {
		for (auto& it : m_properties) {
			delete it.second;
		}
	}

	template<typename T>
	void Set(const std::string& key, const T& value) {
		if (m_properties.find(key) != m_properties.end()) {
			m_properties[key]->Set<T>(value);
		}
		else {
			PropertyBase* base = new Property<T>();
			base->Set<T>(value);
			m_properties[key] = base;
		}
	}

	template<typename T>
	const T& Get(const std::string& key, const T& defaultValue) {
		if (m_properties.find(key) != m_properties.end()) {
			return m_properties[key]->Get<T>();
		}
		else {
			return defaultValue;
		}
	}

private:
	std::map<std::string, PropertyBase*> m_properties;
};