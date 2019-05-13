#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

BytePacker::BytePacker(eEndianness byteOrder /*= LITTLE_ENDIAN*/)
	: m_endianness(byteOrder)
	, m_option(BYTEPACKER_CAN_GROW) {

}

BytePacker::BytePacker(size_t bufferSize, eEndianness byteOrder /*= LITTLE_ENDIAN*/)
	: m_endianness(byteOrder)
	, m_option(BYTEPACKER_OWNS_MEMORY) 
	, m_readableByteCount(bufferSize) {
	m_buffer.resize(bufferSize);
}

BytePacker::BytePacker(size_t bufferSize, void* buffer, eEndianness byteOrder /*= LITTLE_ENDIAN*/) 
	: BytePacker(bufferSize, byteOrder) {
	for(size_t i = 0; i < bufferSize; ++i){
		m_buffer[i] = ((u8*)buffer)[i];
		m_bytesWriteHead++;
	}
}

BytePacker::BytePacker(BytePacker&& other) {
	m_endianness = other.m_endianness;
	m_buffer = std::move(other.m_buffer);
	m_readableByteCount = other.m_readableByteCount;
	m_bytesReadHead = other.m_bytesReadHead;
	m_bytesWriteHead = other.m_bytesWriteHead;
	m_option = other.m_option;

	other.m_readableByteCount = 0;
	other.m_bytesWriteHead = 0;
	other.m_bytesWriteHead = 0;
}

BytePacker& BytePacker::operator=(BytePacker&& other) {
	if (this != &other) {
		m_endianness = other.m_endianness;
		m_buffer = std::move(other.m_buffer);
		m_readableByteCount = other.m_readableByteCount;
		m_bytesReadHead = other.m_bytesReadHead;
		m_bytesWriteHead = other.m_bytesWriteHead;
		m_option = other.m_option;

		other.m_readableByteCount = 0;
		other.m_bytesWriteHead = 0;
		other.m_bytesWriteHead = 0;
	}
	return *this;
}

BytePacker::~BytePacker() {}

void BytePacker::SetEndianness(eEndianness e) {
	m_endianness = e;
}

bool BytePacker::SetReadableByteCount(size_t byteCount) {
	if (byteCount > m_buffer.size()) {
		return false;
	}
	m_readableByteCount = byteCount;
	return true;
}

bool BytePacker::WriteBytes(const void* data, size_t byteCount) {
	if (byteCount > GetWritableByteCount()) {
		LogWarningf("No enough space for writing data.");
		return false;
	}

	for (size_t i = 0; i < byteCount; ++i) {
		u8 byteToWrite = ((u8*)data)[i];
		if (m_option == BYTEPACKER_OWNS_MEMORY) {
			m_buffer[m_bytesWriteHead] = byteToWrite;
		}
		else if (m_option == BYTEPACKER_CAN_GROW) {
			m_buffer.push_back(byteToWrite);
		}
		m_bytesWriteHead++;
	}
	return true;
}

bool BytePacker::WriteBytesAt(size_t start, const void* data, size_t byteCount) {
	if(m_option == BYTEPACKER_OWNS_MEMORY && byteCount > m_buffer.size()){
		return false;
	}

	for(size_t i = 0; i < byteCount; ++i){
		u8 byteToWrite = ((u8*)data)[i];
		if (m_option == BYTEPACKER_OWNS_MEMORY) {
			m_buffer[start + i] = byteToWrite;
		}
		else if (m_option == BYTEPACKER_CAN_GROW) {
#pragma TODO("WriteBytesAt hasn't been fully implemented!")
			ERROR_AND_DIE("WriteBytesAt hasn't been fully implemented!");
		}
	}
	return true;
}

size_t BytePacker::ReadBytes(void* outData, size_t maxByteCount) const{
	for (size_t i = 0; i < maxByteCount; ++i) {
		if (m_bytesReadHead >= m_readableByteCount) {
			return i;
		}
		((u8*)outData)[i] = m_buffer[m_bytesReadHead++];
	}
	return maxByteCount;
}

size_t BytePacker::WriteSize(size_t size) {
	size_t bytesWritten = 0;
	size_t remainder = size;
	while(true) {
		u8 low7Bits = remainder & 0x7F;
		remainder >>= 7;
		bytesWritten++;
		if (remainder > 0) {
			low7Bits |= 0x80;
			WriteBytes(&low7Bits, 1);
		}
		else {
			WriteBytes(&low7Bits, 1);
			break;
		}
	}
	return bytesWritten;
}

size_t BytePacker::ReadSize(size_t* outSize) const{
	std::vector<u8> bytes;
	while (true) {
		u8 byte = 0;
		if(ReadBytes(&byte, 1) <= 0){
			break;
		}
		bytes.push_back(byte);
		if (byte >> 7 == 0) {
			break;
		}
	}
	for (size_t i = 0; i < bytes.size(); ++i) {
		bytes[i] &= 0x7F;
		*outSize |= bytes[i] << (7 * i);
	}
	return bytes.size();
}
 
bool BytePacker::WriteString(const std::string& str) {
	if (WriteSize(str.length()) == 0) return false;
	if (WriteBytes(str.c_str(), str.length()) == 0) return false;
	return true;
}

size_t BytePacker::ReadString(std::string& out) const{
	size_t strLength = 0;
	ReadSize(&strLength);
	for (size_t i = 0; i < strLength; ++i) {
		char byte;
		if (ReadBytes(&byte, 1) == 0) {
			return i;
		}
		out += byte;
	}
	return strLength;
}

void BytePacker::ResetWrite() {
	ResetRead();
	m_bytesWriteHead = 0;
}

void BytePacker::ResetRead() {
	m_bytesReadHead = 0;
}

eEndianness BytePacker::GetEndianness() const {
	return m_endianness;
}

size_t BytePacker::GetWrittenByteCount() const {
	return m_bytesWriteHead;
}

size_t BytePacker::GetWritableByteCount() const {
	if (m_option == BYTEPACKER_OWNS_MEMORY) {
		return m_buffer.size() - GetWrittenByteCount();
	}
	else {
		return UINFINITY;
	}
}

size_t BytePacker::GetReadableByteCount() const {
	return m_bytesWriteHead - m_bytesReadHead;
}

const void* BytePacker::GetBuffer() const{
	return m_buffer.data();
}

