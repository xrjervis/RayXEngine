#include "Engine/Core/Allocator.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cstdlib>

static u32 GetAlignedAddress(u32 originAddress, u32 numAlignBits){
	u32 mask = numAlignBits - 1U;
	u32 toAlign = originAddress & mask;
	u32 finalAddress = originAddress + (mask - toAlign);
	return finalAddress;
}

IAllocator::IAllocator(u32 totalSize)
	: m_totalSize(totalSize) {
}

IAllocator::~IAllocator() {

}

StackAllocator::StackAllocator(u32 totalSize)
	: IAllocator(totalSize) {
	m_pBase = ::malloc(totalSize);
	m_marker = m_pBase;
}

StackAllocator::~StackAllocator() {

}

void* StackAllocator::Alloc(u32 size, u32 numAlignBits /*= 0u*/) {
	u32 currentAddress = *(u32*)m_marker;

	u32 alignedAddress;
	if(numAlignBits > 0U){
		alignedAddress = GetAlignedAddress(currentAddress, numAlignBits);
	}
	else{
		alignedAddress = currentAddress;
	}
	
	m_marker = (void*)(&alignedAddress + size);

	return (void*)&m_marker;
}

void StackAllocator::Free(void* p) {
	m_marker = p;
}