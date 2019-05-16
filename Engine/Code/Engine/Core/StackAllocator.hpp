#pragma once
#include "Engine/Core/IAllocator.hpp"

class StackAllocator : public IAllocator {
public:
	StackAllocator(size_t totalSize);
	virtual ~StackAllocator();

	void* Alloc(size_t size, size_t alignment = 8) override;

	template<typename T>
	T* Alloc(size_t alignment = 8) {

	}

	void FreeTop();
	void Clear() override;

private:
	void Free(void* ptr) override {}

protected:
	void*	m_basePtr = nullptr;
	void*	m_topPtr = nullptr;
	bool	m_isUsed = false;

private:
	struct Header {
		size_t lastBlockStart;
		size_t thisBlockSize;
	};
};
