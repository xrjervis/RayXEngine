#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <unordered_map>
#include <set>

class EventSystem {
public:
	EventSystem();
	~EventSystem();

	void Subscribe(const std::string& eventName, const std::string& funcName);
	void UnSubscribe(const std::string& eventName, const std::string& funcName);
    
	template<typename... Args>
	void FireEvents(const std::string& eventName, Args&&... args);

private:
 	std::unordered_map<std::string, std::set<std::string>> m_subscriptions;
};

template<typename... Args>
void EventSystem::FireEvents(const std::string& eventName, Args&&... args) {
	if (m_subscriptions.find(eventName) != m_subscriptions.end()) {
		for (auto& it : m_subscriptions[eventName]) {
			bool consumeEvents = g_functionLibrary.Invoke<bool>(it, std::forward<Args>(args)...);
			if (consumeEvents) {
				break;
			}
		}
	}
}

extern Uptr<EventSystem> g_theEventSystem;