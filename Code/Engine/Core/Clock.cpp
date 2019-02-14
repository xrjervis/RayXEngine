#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/EngineCommon.hpp"

std::unique_ptr<Clock> g_theMasterClock;

Clock::Clock(Clock* parent /*= nullptr*/) {
	m_parent = parent;
	Reset();
}

Clock::~Clock() {
}

void Clock::Reset() {
	m_lastFrameHPC = GetPerformanceCounter();
	memset(&frame, 0, sizeof(frame));
	memset(&total, 0, sizeof(total));
}

void Clock::BeginFrame() {
	GUARANTEE_OR_DIE( m_parent == nullptr, "Clock::BeginFrame only gets called on root clocks!" );
	u64 curHPC = GetPerformanceCounter();
	u64 elapsed = curHPC - m_lastFrameHPC;
	Advance(elapsed);
	m_lastFrameHPC = curHPC;
}

void Clock::Advance(u64 elapsedHPC) {
	if (m_paused) {
		elapsedHPC = 0;
	}
	else {
		elapsedHPC = (u64)((double)elapsedHPC * m_timeScale);
	}
	m_frameCount++;

	double elapsedSeconds = PerformanceCountToSeconds(elapsedHPC);
	frame.seconds = elapsedSeconds;
	frame.hpc = elapsedHPC;
	frame.mileSeconds = (unsigned int)(frame.seconds * 1000.0);

	total.hpc += frame.hpc;
	total.seconds = PerformanceCountToSeconds(total.hpc);
	total.mileSeconds = (unsigned int)(total.seconds * 1000.0);

	for (auto it : m_children) {
		it->Advance(elapsedHPC);
	}
}

void Clock::AddChild(Clock* child) {
	m_children.emplace_back(child);
}

void Clock::RemoveChild(Clock* child) {
	for (int i = 0; i < m_children.size(); ++i) {
		if (m_children[i] == child) {
			m_children[i] = m_children[m_children.size() - 1];
			m_children.pop_back();
			i--;
		}
	}
}

void Clock::SetPaused(bool paused) {
	m_paused = paused;
	for (auto it : m_children) {
		it->SetPaused(paused);
	}
}

void Clock::SetScale(float scale) {
	m_timeScale = scale;
	for (auto it : m_children) {
		it->SetScale(scale);
	}
}

float Clock::GetDeltaSeconds() const {
	return (float)frame.seconds;
}

Clock* Clock::GetMasterClock() {
	return g_theMasterClock.get();
}

float Clock::GetMasterCurrentSeconds() {
	double seconds = 0.0;
	u64 currentHPC = GetPerformanceCounter();
	seconds = PerformanceCountToSeconds(currentHPC - g_theMasterClock->GetStartHPC());
	return (float)seconds;
}

float Clock::GetMasterDeltaSeconds() {
	return g_theMasterClock->GetDeltaSeconds();
}
