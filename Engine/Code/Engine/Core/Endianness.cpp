#include "Engine/Core/Endianness.hpp"
#include "Engine/Core/type.hpp"
#include <utility>

eEndianness GetPlatformEndianness() {
	const u32 num = 1;
	const u8* array = (const u8*)&num;
	if (array[0] == 0x01) {
		return LITTLE_ENDIAN;
	}
	else {
		return BIG_ENDIAN;
	}
}

void ToEndianness(const size_t size, void* data, eEndianness endianness) {
	if (GetPlatformEndianness() == endianness) {
		return;
	}

	u8* byteBuffer = (u8*)data;

	uint i = 0;
	uint j = (uint)size - 1U;
	while (i < j) {
		std::swap(byteBuffer[i], byteBuffer[j]);
		++i;
		--j;
	}
}

void FromEndianness(const size_t size, void* data, eEndianness endianness) {
	if (GetPlatformEndianness() == endianness) {
		return;
	}

	u8* byteBuffer = (u8*)data;

	uint i = 0u;
	uint j = (uint)(size - 1U);
	while (i < j) {
		std::swap(byteBuffer[i], byteBuffer[j]);
		++i;
		--j;
	}
}
