#pragma once
#include "Engine/Core/type.hpp"
#include <thread>
#include <memory>
#include <vector>
#include <future>
#include <chrono>

constexpr int MAX_THREADS = 8;

class ThreadManager {
public:
	ThreadManager() = default;
	~ThreadManager() = default;

	template <typename Func, typename... Args>
	threadHandle CreateThread(Func&& f, Args&&... args) {
		for (uint i = 0; i < MAX_THREADS; ++i) {
			if (m_threads[i] == nullptr || !IsRunning(i)) {
				m_threads[i] = std::make_unique<std::thread>(std::forward<Func>(f), std::forward<Args>(args)...);
				return (threadHandle)i;
			}
		}
		return -1;
	}

	void Join(threadHandle handle);
	void Detach(threadHandle handle);
	bool IsRunning(threadHandle handle);

private:
	std::unique_ptr<std::thread> m_threads[MAX_THREADS];
};

extern ThreadManager g_theThreadManager;
