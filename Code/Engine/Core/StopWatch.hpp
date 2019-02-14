#pragma once
#include "Engine/Core/type.hpp"
#include <cstdint>

class Clock;

class StopWatch {
public:
	StopWatch(Clock* refClock = nullptr);
	~StopWatch();

	void SetClock(Clock* refClock = nullptr);

	bool SetTimer(float seconds);

	float GetElapsedTime() const;

	//[0.f, 1.f]
	float GetNormalizedElapsedTime() const;

	bool HasElapsed() const;

	void Reset();

	// Checks the interval, if it has elapsed, will
	// return true and reset.  If not, will just return false (no reset)
	bool CheckAndReset();

	// returns has_elapsed(), and if so
	// will subtract one interval from 
	// the elapsed time (but maintaining 
	// any extra time we have accrued)
	//bool Decrement();

	// like decrement, but returns how many 
	// times we have elapsed our timer
	// so say our timer was 0.5 seconds, 
	// and get_elapsed_time() was 2.2s.  
	// decrement_all() would return 4, and 
	// update itself so that get_elapsed_time()
	// would return 0.2s;
	//uint DecrementAll();

	//void Pause();
	//void Resume();

public:
	Clock* m_referenceClock = nullptr;
	uint64_t m_startHPC;
	uint64_t m_intervalHPC;
};