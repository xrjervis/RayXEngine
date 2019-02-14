#pragma once
#include <mutex>
#include <queue>
#include <set>

class SpinLock {
public:
	void Enter() { m_lock.lock(); } // blocking operation
	bool TryEnter() { return m_lock.try_lock(); }
	void Exit() { m_lock.unlock(); } // must be called if entered

public:
	std::mutex m_lock;
};

template <typename T>
class ThreadSafeQueue {
public:
	void Enqueue(T&& v) noexcept {
		m_lock.Enter();

		// I AM THE ONLY PERSON HERE
		m_data.emplace(v);

		m_lock.Exit();
		// no longer true...
	}

	// return if it succeeds
	bool Dequeue(T& out) {
		m_lock.Enter();

		bool hasItem = !m_data.empty();
		if (hasItem) {
			out = std::move(m_data.front());
			m_data.pop();
		}

		m_lock.Exit();
		return hasItem;
	}

public:
	std::queue<T> m_data;
	SpinLock m_lock;
};

template <typename T>
class ThreadSafeSet {
public:
	void Insert(const T& v) noexcept {
		m_lock.Enter();
		m_data.emplace(v);
		m_lock.Exit();
	}

	void Erase(const T& e) {
		m_lock.Enter();
		for (auto& it : m_data) {
			if (it == e) {
				m_data.erase(it);
			}
		}
		m_lock.Exit();
	}

	bool Find(const T& e) {
		m_lock.Enter();
		bool hasFound = m_data.find(e) != m_data.end();
		m_lock.Exit();
		return hasFound;
	}

	void Clear() {
		m_lock.Enter();
		m_data.clear();
		m_lock.Exit();
	}

public:
	std::set<T> m_data;
	SpinLock m_lock;
};