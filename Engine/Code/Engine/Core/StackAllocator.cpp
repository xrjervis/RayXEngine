#include "Engine/Core/StackAllocator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

StackAllocator::StackAllocator(size_t totalSize)
	: IAllocator(totalSize)
	, m_isUsed(false) {
	m_basePtr = AllocAligned(totalSize, 8);
	m_topPtr = m_basePtr;
}

StackAllocator::~StackAllocator() {
	FreeAligned(m_basePtr);
	m_basePtr = nullptr;
	m_topPtr = nullptr;
	m_isUsed = false;
}

void* StackAllocator::Alloc(size_t size, size_t alignment) {
	Header h;

	if (m_isUsed == false) {
		m_isUsed = true;
		h.lastBlockStart = (size_t)m_basePtr;
		h.thisBlockSize = size + sizeof(Header);
		*((Header*)m_topPtr) = h;
		return (void*)((size_t)m_topPtr + sizeof(Header));
	}
	else {
		Header* topBlockHeader = (Header*)(m_topPtr);
		size_t nextAddress = (size_t)m_topPtr + topBlockHeader->thisBlockSize;
		if (nextAddress + size + sizeof(Header) - (size_t)m_basePtr > m_totalSize) {
			DebuggerPrintf("Alloc failed: stack allocator overflow!");
			return nullptr;
		}
		size_t alignedAddress = AlignAddress(nextAddress, alignment);

		h.lastBlockStart = (size_t)m_topPtr;
		h.thisBlockSize = size + sizeof(Header);

		void* nextPtr = (void*)nextAddress;
		*((Header*)nextPtr) = h;
		m_topPtr = nextPtr;

		return (void*)(nextAddress + sizeof(Header));
	}
}

void StackAllocator::FreeTop() {
	if (m_topPtr == m_basePtr) {
		if (m_isUsed) {
			m_isUsed = false;
		}
		else {
			DebuggerPrintf("FreeTop failed: stack allocator is already empty!");
		}
	}
	else {
		Header h;
		h = *((Header*)m_topPtr);
		size_t lastAddress = h.lastBlockStart;
		m_topPtr = (void*)lastAddress;
	}
}

void StackAllocator::Clear() {
	IAllocator::Clear();
	m_topPtr = m_basePtr;
	m_isUsed = false;
}

