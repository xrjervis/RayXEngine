#pragma once

enum eEndianness {
	LITTLE_ENDIAN,
	BIG_ENDIAN,
};

eEndianness GetPlatformEndianness();

// Assumes data is in platform endianness, and will convert to supplied endianness; 
void ToEndianness(const size_t size, void* data, eEndianness endianness);

// Assumes data is in supplied endianness, and will convert to platform's endianness
void FromEndianness(const size_t size, void* data, eEndianness endianness);