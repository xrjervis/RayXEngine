#pragma once
#include "Engine/Core/IAllocator.hpp"

class StackAllocator : public IAllocator {
public:
	StackAllocator(size_t totalSize);
	virtual ~StackAllocator();

	void* Alloc(size_t size, size_t alignment) override;
	void FreeToMarker();

protected:
	void*	m_basePtr = nullptr;
	size_t		m_offset = 0;
};
