#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <mutex>

template <typename T>
class TripleBuffer{
public:
	TripleBuffer() = default;
	~TripleBuffer() = default;

	T*		BeginWrite();
	void	EndWrite();
	T*		Read();

// 	T*		LockForWrite(); 
// 	void	EndWrite();
// 
// 	T const* LockForRead(); 
// 	void UnlockRead(); 

private:
	T m_buffer[3];
	int dirty = 0;
	int clean = 1;
	int ready = -1;
	bool hasNewWrite = false;

	std::mutex m_mutex;
};

template <typename T>
T* TripleBuffer<T>::Read() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (hasNewWrite) {
		if(ready == -1){
			// only for the first ready data
			ready = 2;
		}
		std::swap(clean, ready);
		hasNewWrite = false;
	}
	else{
		if (ready == -1) {
			return nullptr;
		}
	}
	return &m_buffer[ready];
}

template <typename T>
void TripleBuffer<T>::EndWrite() {
	std::unique_lock<std::mutex> lock(m_mutex);
	hasNewWrite = true;
	std::swap(dirty, clean);
}

template <typename T>
T* TripleBuffer<T>::BeginWrite() {
	std::unique_lock<std::mutex> lock(m_mutex);
	return &m_buffer[dirty];
}

