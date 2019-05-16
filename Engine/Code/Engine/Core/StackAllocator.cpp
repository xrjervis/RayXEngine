#include "Engine/Core/StackAllocator.hpp"

StackAllocator::StackAllocator(size_t totalSize)
	: IAllocator(totalSize) 
	, m_offset(0U) {
	m_basePtr = ::malloc(m_totalSize);
}

StackAllocator::~StackAllocator() {
	::free(m_basePtr);
	m_basePtr = nullptr;
}

void* StackAllocator::Alloc(size_t size, size_t alignment) {
	size_t currentAddress = (size_t)m_basePtr + m_offset;
	u8 padding = currentAddress % alignment;


}

void StackAllocator::Free(void* ptr) {

}
