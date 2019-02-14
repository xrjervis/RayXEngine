#pragma once
#include "Engine/Core/type.hpp"

// supports alignment
class IAllocator {
public:
	IAllocator(u32 totalSize);
	virtual ~IAllocator();

	virtual void*	Alloc(u32 size, u32 numAlignBits = 0u) = 0;
	virtual void	Free(void* p = nullptr) = 0;

protected:
	u32 m_totalSize = 0U;
	void* m_pBase = nullptr;

public:
	template <typename T, typename ...ARGS>
	T* Create(ARGS ...args) {
		void* buffer = Alloc(sizeof(T));
		return new (buffer) T(args...);
	}
	
	template <typename T>
	void Destroy(T *obj) {
		obj->~T();
		Free(obj);
	}
};

class StackAllocator : public IAllocator {
public:
	explicit StackAllocator(u32 totalSize);
	virtual ~StackAllocator();

	virtual void* Alloc(u32 size, u32 numAlignBits = 0u) override;
	virtual void Free(void* p = nullptr) override;

private:
	void* m_marker = nullptr;
};