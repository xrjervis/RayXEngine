#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <stdio.h>
#include <filesystem>

char* FileSystem::FileReadToBuffer(const char* filename) {
	std::ifstream is(filename, std::ifstream::binary);
	if (is) {
		// get length of file:
		is.seekg(0, is.end);
		int length = (int)is.tellg();
		is.seekg(0, is.beg);

		// allocate memory:
		char* buffer = new char[length + 1];

		// read data as a block:
		is.read(buffer, length);
		buffer[length] = '\0';

		is.close();

		return buffer;
	}
	else {
		ERROR_AND_DIE(Stringf("%s doesn't exist!", filename));
		return nullptr;
	}
}

void FileSystem::BufferWriteToFile(const char* filename, char* buffer, size_t bufferSize, bool truncate, bool startAtEnd, bool binary) {
	std::fstream fs;

	if (truncate && binary) {
		fs.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	}
	else if (startAtEnd && binary) {
		fs.open(filename, std::ios::out | std::ios::app | std::ios::binary);
	}
	else {
		fs.open(filename, std::ios::out);
	}

	fs.write(buffer, bufferSize);
	fs.close();
}

void FileSystem::WriteToFile(std::fstream& fout, const std::string& data) {
	if (!fout.is_open()) {
		ERROR_AND_DIE("file is not open!");
	}
	else {
		fout << data;
	}
}

bool FileSystem::Exists(const std::wstring& filePath) {
	std::experimental::filesystem::path path = filePath;
	return std::experimental::filesystem::exists(path);
}

bool FileSystem::Exists(const std::string& filePath) {
	std::experimental::filesystem::path path = filePath;
	return std::experimental::filesystem::exists(path);
}
