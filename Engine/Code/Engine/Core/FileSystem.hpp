#pragma once
#include <fstream>
#include <string>

class FileSystem {
public:
	FileSystem() = default;
	~FileSystem() = default;

	static char* FileReadToBuffer(const char* filename);
	static void BufferWriteToFile(const char* filename, char* buffer, size_t bufferSize, bool truncate, bool startAtEnd, bool binary);
	static void WriteToFile(std::fstream& fout, const std::string& data);
	static bool Exists(const std::wstring& filePath);
	static bool Exists(const std::string& filePath);

};