#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"

StopWatch::StopWatch(Clock* refClock /*= nullptr*/) {
	SetClock(refClock);
	Reset();
}

StopWatch::~StopWatch() {

}

void StopWatch::SetClock(Clock* refClock /*= nullptr*/) {
	m_referenceClock = refClock;
}

bool StopWatch::SetTimer(float seconds) {
	m_intervalHPC = SecondsToPerformanceCount(seconds);
	return true;
}

float StopWatch::GetElapsedTime() const {
	uint64_t elapsedHPC = m_referenceClock->GetCurrentHPC() - m_startHPC;
	float elapsedSeconds = (float)PerformanceCountToSeconds(elapsedHPC);
	return elapsedSeconds;
}

float StopWatch::GetNormalizedElapsedTime() const {
	uint64_t elapsedHPC = m_referenceClock->GetCurrentHPC() - m_startHPC;
	float normalizedElapsed = (float)elapsedHPC / (float)m_intervalHPC;
	return normalizedElapsed;
}

bool StopWatch::HasElapsed() const {
	uint64_t elapsedHPC = m_referenceClock->GetCurrentHPC() - m_startHPC;
	if (elapsedHPC >= m_intervalHPC) {
		return true;
	}
	else {
		return false;
	}
}

void StopWatch::Reset() {
	m_startHPC = m_referenceClock->GetCurrentHPC();
}

bool StopWatch::CheckAndReset() {
	if (HasElapsed()) {
		Reset();
		return true;
	}
	else {
		return false;
	}
}

