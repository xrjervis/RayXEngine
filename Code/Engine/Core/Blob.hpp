#pragma once
class Blob {
public:
	Blob();
	~Blob();

	// copy semantics
	Blob(const Blob& copy) = delete;
	Blob& operator=(const Blob& copy);

	// move semantics
	Blob(Blob&& move);
	Blob& operator=(Blob&& move);

	// updates the blob to be a copy of the provided memory.
	bool CopyFrom(const void* src, const size_t byteCount);

	// update a section of the buffer located at offset return 
	//false if this would not fit
	bool Update(const void* src, const size_t byteCount, const size_t offset);

	// resizes the buffer, maintaining stored memory
	// if made smaller, should truncate.  If made larger
	// should leave the newly allocated space as garbage.
	bool Resize(const size_t newSize);

	inline const void* GetBufferPointer() const { return m_buffer; }
	inline size_t GetBufferSize() const { return m_bufferSize; }
private:
	void*	m_buffer = nullptr;
	size_t	m_bufferSize;
};
