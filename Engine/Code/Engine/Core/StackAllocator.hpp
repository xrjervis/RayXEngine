#pragma once
#include "Engine/Core/IAllocator.hpp"

class StackAllocator {
public:
	StackAllocator(size_t totalSize);
	virtual ~StackAllocator();

	void* Alloc(size_t size, size_t alignment = 8) ;
	void Free(void* ptr);  // will free all the way up to the top
	void Clear() ;

private:
	size_t  m_totalSize = 0;
	void*	m_basePtr = nullptr;
	void*	m_topPtr = nullptr;
};
