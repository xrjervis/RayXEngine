#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/Endianness.hpp"
#include <vector>

enum eBytePackerOption{
	BYTEPACKER_OWNS_MEMORY,
	BYTEPACKER_CAN_GROW
};

// allows both copy and move constructor
class BytePacker {
public:
	BytePacker(eEndianness byteOrder = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, eEndianness byteOrder = LITTLE_ENDIAN);
	BytePacker(size_t bufferSize, void* buffer, eEndianness byteOrder = LITTLE_ENDIAN);
	BytePacker(const BytePacker& other) = default;
	BytePacker& operator=(const BytePacker& other) = default;
	BytePacker(BytePacker&& other);
	BytePacker& operator=(BytePacker&& other);
	virtual ~BytePacker();

	void	SetEndianness(eEndianness e);
	bool	SetReadableByteCount(size_t byteCount);

	// tries to write data to the end of the buffer;  Returns success
	bool	WriteBytes(const void* data, size_t byteCount);
	bool    WriteBytesAt(size_t start, const void* data, size_t byteCount);
	// Tries to read into out_data.  Returns how much ended up being read; 
	size_t	ReadBytes(void* outData, size_t maxByteCount) const;
	// returns how many bytes used
	size_t	WriteSize(size_t size);
	// returns how many bytes read, fills out_size
	size_t	ReadSize(size_t* outSize) const;

	// Write encoded string
	bool	WriteString(const std::string& str);
	size_t	ReadString(std::string& out) const; 

	// resets writing to the beginning of the buffer.  Make sure read head stays valid (<= write_head)
	void	ResetWrite(); 
	// resets reading to the beginning of the buffer
	void	ResetRead();   

	eEndianness GetEndianness() const;
	size_t		GetWrittenByteCount() const;    // how much have I written to this buffer
	size_t		GetWritableByteCount() const;   // how much more can I write to this buffer (if growable, this returns UINFINITY)
	size_t		GetReadableByteCount() const;   // how much more data can I read;
	const void*	GetBuffer() const;

private:
	eEndianness m_endianness;
	std::vector<u8> m_buffer;
	mutable size_t m_readableByteCount = 0;
	mutable size_t m_bytesReadHead = 0;
	mutable size_t m_bytesWriteHead = 0;
	eBytePackerOption m_option;
};