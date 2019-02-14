#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;

using ComponentTypeID = size_t;

inline ComponentTypeID GetComponentTypeID() {
	static ComponentTypeID lastID = 0;
	return lastID++;
}

template<typename T>
inline ComponentTypeID GetComponentTypeID() noexcept {
	static ComponentTypeID typeID = GetComponentTypeID();
	return typeID;
}

constexpr size_t MAX_COMPONENTS = 32;

using ComponentBitset = std::bitset<MAX_COMPONENTS>;
using ComponentArray = std::array<Component*, MAX_COMPONENTS>;

class Component {
public:
	virtual ~Component() = default;
	virtual void Init() = 0;
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() = 0;

public:
	Entity* m_entity;
};

class Entity {
public:
	void Update(float deltaSeconds) {
		for (auto& c : m_components) {
			c->Update(deltaSeconds);
		}
	}

	void Render() {
		for (auto& c : m_components) {
			c->Render();
		}
	}

	bool IsActive() {
		return m_isActive;
	}

	void Destroy() {
		m_isActive = false;
	}

	template<typename T>
	bool HasComponent() const {
		return m_componentBitset[GetComponentTypeID<T>()];
	}

	template<typename T, typename... TArgs>
	T& AddComponent(TArgs&&... args) {
		Uptr<T> c = std::make_unique<T>(std::forward<TArgs>(args));
		c->m_entity = this;
		T* ptr = c.get();
		m_components.emplace_back(std::move(c));

		ComponentTypeID typeID = GetComponentTypeID<T>();
		m_componentArray[typeID] = ptr;
		m_componentBitset[typeID] = true;

		ptr->Init();
		return *ptr;
	}

	template<typename T>
	T& GetComponent() const {
		ComponentTypeID typeID = GetComponentTypeID<T>();
		auto ptr(m_componentArray[typeID]);
		return *static_cast<T*>(ptr);
	}

private:
	bool m_isActive = true;
	std::vector<Uptr<Component>>	m_components;

	ComponentArray					m_componentArray;
	ComponentBitset					m_componentBitset;
};

class Manager {
public:
	void Update(float deltaSeconds) {
		for (auto& e : m_entities) {
			e->Update(float deltaSeconds);
		}
	}

	void Render() {
		for (auto& e : m_entities) {
			e->Render();
		}
	}

	void Refresh() {
		m_entities.erase(std::remove_if(std::begin(m_entities), 
			std::end(m_entities), 
			[](const Uptr<Entity>& e) {
			return !e->IsActive();
			}), std::end(m_entities));
	}

	Entity& AddEntity() {
		Uptr<Entity> e = std::make_unique<Entity>();
		Entity* ptr = e.get();
		m_entities.emplace_back(std::move(e));
		return *ptr;
	}

private:
	std::vector<Uptr<Entity>> m_entities;
};