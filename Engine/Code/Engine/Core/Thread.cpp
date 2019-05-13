#include "Engine/Core/Thread.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

ThreadManager g_theThreadManager;

void ThreadManager::Join(threadHandle handle) {
	if (handle >= 0 && m_threads[handle]->joinable()) {
		m_threads[handle]->join();
	}
	else {
		DebuggerPrintf("ThreadHandle is invalid!\n");
	}
}

void ThreadManager::Detach(threadHandle handle) {
	if (handle >= 0 && m_threads[handle]->joinable()) {
		m_threads[handle]->detach();
	}
	else {
		DebuggerPrintf("ThreadHandle is invalid!\n");
	}
}

bool ThreadManager::IsRunning(threadHandle handle) {
	if (handle >= 0) {
		return m_threads[handle]->joinable();
	}
	else {
		DebuggerPrintf("ThreadHandle is invalid!\n");
		return false;
	}
}
