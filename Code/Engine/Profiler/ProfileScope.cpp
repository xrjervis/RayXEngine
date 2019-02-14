#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"

LogProfileScope::LogProfileScope(const std::string& tag) {
	m_tag = tag;
	m_startHPC = GetPerformanceCounter();
}

LogProfileScope::~LogProfileScope() {
	u64 elapsed = GetPerformanceCounter() - m_startHPC;
	DebuggerPrintf("[%s] took %.2f seconds.\n", m_tag.c_str(), PerformanceCountToSeconds(elapsed));
}

ProfileScope::ProfileScope(const std::string& tag) 
	:m_tag(tag) {
	g_theProfiler->Push(tag);
}

ProfileScope::~ProfileScope() {
	g_theProfiler->Pop();
}
