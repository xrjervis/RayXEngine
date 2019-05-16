#pragma once
#include "Engine/Core/type.hpp"

class IAllocator {
public:
	IAllocator(size_t totalSize) : m_totalSize(totalSize) {}
	virtual ~IAllocator() {}

	virtual void* Alloc(size_t size, size_t alignment) = 0;
	
	virtual void Free(void* ptr) {
	}
	
	virtual void Clear() { 
		m_totalSize = 0; 
	}

protected:
	size_t m_totalSize = 0;

};

inline size_t AlignAddress(size_t addr, size_t alignment) {
	size_t mask = alignment - 1;

	/*To strip off these bits, we need a mask that we can apply to the address using the bitwise AND operator. Because L is always a power of two, L - 1 will be a mask with binary 1s in the N least-significant bits and binary 0s everywhere else. So all we need to do is invert this mask and then AND it with the address(addr & ~mask).*/
	return (addr + mask) & (~mask);
}

template<typename T>
inline T* AlignPointer(T* ptr, size_t alignment) {
	const size_t addr = reinterpret_cast<size_t>(ptr);
	const size_t addrAligned = AlignAddress(addr, alignment);
	return reinterpret_cast<T*>(addrAligned);
}

// alignment must be a power of 2
inline void* AllocAligned(size_t bytes, size_t alignment) {
	// work for up to 256-byte alignment
	/*we simply allocate L extra bytes, instead of L - 1, and then we always shift the raw pointer up to the next L-byte boundary, even if it was already aligned.*/
	size_t worstCaseBytes = bytes + alignment;

	u8* rawPtr = new u8[worstCaseBytes];

	// Align the block. If no alignment occurred,
	// shift it up the full alignment bytes
	u8* alignedPtr = AlignPointer(rawPtr, alignment);
	if (alignedPtr == rawPtr) {
		alignedPtr += alignment;
	}

	size_t shift = alignedPtr - rawPtr;
	alignedPtr[-1] = static_cast<u8>(shift & 0xFF);

	return alignedPtr;
}

inline void FreeAligned(void* ptr) {
	if (ptr) {
		u8* alignedPtr = reinterpret_cast<u8*>(ptr);
		size_t shift = alignedPtr[-1];
		if (shift == 0) {
			shift = 256;
		}

		u8* rawPtr = alignedPtr - shift;
		delete[] rawPtr;
	}
}