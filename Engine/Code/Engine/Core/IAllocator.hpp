#pragma once
#include "Engine/Core/type.hpp"

class IAllocator {
public:
	IAllocator(u32 totalSize) : m_totalSize(totalSize) {}
	virtual ~IAllocator() = 0;

	virtual void* Alloc(u32 size, u32 alignment) = 0;
	virtual void Free(void* ptr) = 0;

protected:
	u32 m_totalSize = 0U;

};