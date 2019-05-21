#include "Engine/Core/StackAllocator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

StackAllocator::StackAllocator(size_t totalSize) 
	: m_totalSize(totalSize){
	m_basePtr = AllocAligned(totalSize, 8);
	m_topPtr = m_basePtr;
	FillWithPattern(m_basePtr, totalSize, PATTERN_FREE);
}

StackAllocator::~StackAllocator() {
	FreeAligned(m_basePtr);
	m_basePtr = nullptr;
	m_topPtr = nullptr;
}

void* StackAllocator::Alloc(size_t size, size_t alignment) {
	size_t currentAddress = (size_t)m_topPtr;
	size_t alignedAddress = AlignAddress(currentAddress, alignment);
	FillWithPattern(m_topPtr, alignedAddress - currentAddress, PATTERN_ALIGN);
	m_topPtr = (void*)(alignedAddress + size);
	FillWithPattern((void*)alignedAddress, size, PATTERN_ALLOC);
	return (void*)alignedAddress;
}

void StackAllocator::Free(void* ptr) {
	if ((size_t)ptr >= (size_t)m_basePtr && (size_t)ptr <= (size_t)m_topPtr) {
		m_topPtr = ptr;
		FillWithPattern(m_topPtr, m_totalSize - (size_t)m_topPtr + (size_t)m_basePtr, PATTERN_FREE);
	}
}

void StackAllocator::Clear() {
	m_topPtr = m_basePtr;
	FillWithPattern(m_basePtr, m_totalSize, PATTERN_FREE);
}

