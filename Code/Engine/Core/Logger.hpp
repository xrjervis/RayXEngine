#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/ThreadSafeContainer.hpp"
#include <fstream>
#include <string>
#include <vector>

struct Log_t {
	std::string m_tag;
	std::string m_text;
};

typedef void(*log_cb)(const Log_t& data);

class Logger {
public:
	Logger();
	~Logger();

	void LoggerThreadWorker();

	//Hooking
	void Hook(log_cb cb);
	void UnHook(log_cb cb);

public:
	ThreadSafeQueue<Log_t>	m_queue;
	ThreadSafeSet<std::string> m_filter;
	bool					m_isFilterWhiteList = false;
	std::vector<log_cb>		m_hooks;
	std::ofstream			m_logFileStream;
	std::ofstream			m_currentLogFileStream;
	bool					m_isFlushing = false;


private:
	std::string				m_filePath = "Log/log";
	threadHandle			m_thread;
	bool					m_isRunning = true;
	SpinLock				m_hookLock;
};

void LogTaggedPrintv(const std::string& tag, const char* format, va_list args);
void LogTaggedPrintf(const std::string& tag, const char* format, ...);
void LogPrintf(const char* format, ...);
void LogWarningf(const char* format, ...);
void LogErrorf(const char* format, ...);
void LogFlush();
void LogShowAll();
void LogHideAll();
void LogShowTag(const std::string& tag);
void LogHideTag(const std::string& tag);
bool LogTagFilter(const std::string& tag);

void WriteToFile_cb(const Log_t& data);
void WriteToConsole_cb(const Log_t& data);
void WriteToOutputWindow_cb(const Log_t& data);

