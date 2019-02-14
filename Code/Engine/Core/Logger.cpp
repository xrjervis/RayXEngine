#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <algorithm>

constexpr int LOGGER_STRINGF_LENGTH = 4096;

static bool Command_LogFlushTest(Command& cmd) {
	if (!cmd.m_args.empty()) {
		return false;
	}
	else {
		LogPrintf("LogFlushTest");
		LogFlush();
		return true;
	}
}

static bool Command_LogShowAll(Command& cmd) {
	if (!cmd.m_args.empty()) {
		return false;
	}
	else {
		LogShowAll();
		return true;
	}
}

static bool Command_LogHideAll(Command& cmd) {
	if (!cmd.m_args.empty()) {
		return false;
	}
	else {
		LogHideAll();
		return true;
	}
}

static bool Command_LogShowTag(Command& cmd) {
	if (!cmd.m_args.empty()) {
		std::string tag;
		cmd.GetNextArg<std::string>(tag);
		LogShowTag(tag);
		return true;
	}
	else {
		return false;
	}
}

static bool Command_LogHideTag(Command& cmd) {
	if (!cmd.m_args.empty()) {
		std::string tag;
		cmd.GetNextArg<std::string>(tag);
		LogHideTag(tag);
		return true;
	}
	else {
		return false;
	}
}

Logger::Logger()  {
// 	Hook(WriteToFile_cb);
 	Hook(WriteToConsole_cb);
// 	Hook(WriteToOutputWindow_cb);

	if (!m_hooks.empty()) {
		if(std::find(m_hooks.begin(), m_hooks.end(), WriteToFile_cb) != m_hooks.end()){
			m_logFileStream.open(m_filePath + ".txt", std::ofstream::out | std::ofstream::app);
			std::string filepathTimestamp = m_filePath + '_' + GetTimestamp() + ".txt";
			m_currentLogFileStream.open(filepathTimestamp, std::ofstream::out);
			if (m_logFileStream.is_open() == false) {
				CreateDirectoryA("Log", NULL);
				m_logFileStream.open(m_filePath + ".txt", std::ofstream::out | std::ofstream::app);
				m_currentLogFileStream.open(m_filePath + '_' + GetTimestamp() + ".txt", std::ofstream::out);
			}
		}
		m_thread = g_theThreadManager.CreateThread(&Logger::LoggerThreadWorker, this);
	}

	CommandDefinition::Register("log_flush_test", "[N/A] Log flush test.", Command_LogFlushTest);
	CommandDefinition::Register("log_show_all", "[N/A] Show all logs.", Command_LogShowAll);
	CommandDefinition::Register("log_hide_all", "[N/A] Hide all logs.", Command_LogHideAll);
	CommandDefinition::Register("log_show_tag", "[string] Show logs with tag.", Command_LogShowTag);
	CommandDefinition::Register("log_hide_tag", "[string] Hide logs with tag.", Command_LogHideTag);
}

Logger::~Logger() {
	m_isRunning = false;
	if (!m_hooks.empty()) {
		g_theThreadManager.Join(m_thread);
	}
}

void Logger::LoggerThreadWorker() {
	while (m_isRunning) {
		Log_t data;
		while (m_queue.Dequeue(data)) {
			// do something with the data
			m_hookLock.Enter();
			for (auto& hook : m_hooks) {
				hook(data);
			}
			m_hookLock.Exit();
		}
		if (m_isFlushing) {
			m_logFileStream.flush();
			m_isFlushing = false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Logger::Hook(log_cb cb) {
	if (std::find(m_hooks.begin(), m_hooks.end(), cb) == m_hooks.end()) {
		m_hooks.push_back(cb);
	}
}


void Logger::UnHook(log_cb cb) {
	m_hooks.erase(std::remove(m_hooks.begin(), m_hooks.end(), cb), m_hooks.end());
}

void LogTaggedPrintv(const std::string& tag, const char* format, va_list args) {
	char textLiteral[LOGGER_STRINGF_LENGTH];
	vsnprintf_s(textLiteral, LOGGER_STRINGF_LENGTH, _TRUNCATE, format, args);
	textLiteral[LOGGER_STRINGF_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string text(textLiteral);

	Log_t data;
	data.m_tag = tag;
	data.m_text = text;
	g_theLogger->m_queue.Enqueue(std::move(data));
}

void LogTaggedPrintf(const std::string& tag, const char* format, ...) {
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);
	va_end(variableArgumentList);
}

void LogPrintf(const char* format, ...) {
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv("log", format, variableArgumentList);
	va_end(variableArgumentList);
}

void LogWarningf(const char* format, ...) {
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv("warning", format, variableArgumentList);
	va_end(variableArgumentList);
}

void LogErrorf(const char* format, ...) {
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv("error", format, variableArgumentList);
	va_end(variableArgumentList);
}

void LogFlush() {
	g_theLogger->m_isFlushing = true;
	while (g_theLogger->m_isFlushing == true) {}
}

void LogShowAll() {
	g_theLogger->m_isFilterWhiteList = false;
	g_theLogger->m_filter.Clear();
}

void LogHideAll() {
	g_theLogger->m_isFilterWhiteList = true;
	g_theLogger->m_filter.Clear();
}

void LogShowTag(const std::string& tag) {
	if (g_theLogger->m_isFilterWhiteList) {
		g_theLogger->m_filter.Insert(tag);
	}
	else {
		g_theLogger->m_filter.Erase(tag);
	}
}

void LogHideTag(const std::string& tag) {
	if (g_theLogger->m_isFilterWhiteList) {
		g_theLogger->m_filter.Erase(tag);
	}
	else {
		g_theLogger->m_filter.Insert(tag);
	}
}

bool LogTagFilter(const std::string& tag) {
	if (g_theLogger->m_isFilterWhiteList) {
		if (!g_theLogger->m_filter.Find(tag)) {
			return true;
		}
	}
	else {
		if (g_theLogger->m_filter.Find(tag)) {
			return true;
		}
	}
	return false;
}

void WriteToFile_cb(const Log_t& data) {
	if (LogTagFilter(data.m_tag)) {
		return;
	}

	FileSystem::WriteToFile(g_theLogger->m_logFileStream, 
		Stringf("%s [%s]: %s", GetTimestamp().c_str(), data.m_tag.c_str(), data.m_text.c_str()));
	FileSystem::WriteToFile(g_theLogger->m_currentLogFileStream,
		Stringf("%s [%s]: %s", GetTimestamp().c_str(), data.m_tag.c_str(), data.m_text.c_str()));
}

void WriteToConsole_cb(const Log_t& data) {
	if (LogTagFilter(data.m_tag)) {
		return;
	}

	const std::string& tag = data.m_tag;
	Rgba textColor = Rgba::GREEN;
	if (tag == "error") {
		textColor = Rgba::RED;
	}
	else if (tag == "warning") {
		textColor = Rgba::YELLOW;
	}
	ConsolePrintf(textColor, "%s [%s]: %s", GetTimestamp().c_str(), data.m_tag.c_str(), data.m_text.c_str());
}

void WriteToOutputWindow_cb(const Log_t& data) {
	if (LogTagFilter(data.m_tag)) {
		return;
	}
	if (IsDebuggerAvailable()) {
		OutputDebugStringA(Stringf("%s [%s]: %s", GetTimestamp().c_str(), data.m_tag.c_str(), data.m_text.c_str()).c_str());
	}
}