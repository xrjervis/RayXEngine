#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <sstream>
//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

//Exception: boolean
template <class T>
bool ParseFromString(T& out, const std::string& str) {
	T toValue;
	std::stringstream ss(str);
	if (ss >> toValue) {
		out = toValue;
		return true;
	}
	return false;
}
