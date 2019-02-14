#pragma once
#include <fstream>
#include <string>

class FileSystem {
public:
	FileSystem() = default;
	~FileSystem() = default;

	static char* FileReadToBuffer(const char* filename);
	static void WriteToFile(std::ofstream& fout, const std::string& data);
	static bool Exists(const std::wstring& filePath);
};