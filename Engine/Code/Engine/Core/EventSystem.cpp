#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Uptr<EventSystem> g_theEventSystem;

EventSystem::EventSystem() {

}

EventSystem::~EventSystem() {

}

void EventSystem::Subscribe(const std::string& eventName, const std::string& funcName) {
	m_subscriptions[eventName].insert(funcName);
}

void EventSystem::UnSubscribe(const std::string& eventName, const std::string& funcName) {
	if (m_subscriptions.find(eventName) != m_subscriptions.end()) {
		m_subscriptions[eventName].erase(funcName);
	}
}
