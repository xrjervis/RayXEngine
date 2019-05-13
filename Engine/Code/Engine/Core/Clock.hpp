#pragma once
#include "Engine/Core/type.hpp"
#include <cstdint>
#include <vector>
#include <memory>
struct TimeUnit_t {
public:
	TimeUnit_t() { memset(this, 0, sizeof(TimeUnit_t)); }
	uint64_t hpc;
	unsigned mileSeconds;  
	double seconds;      // convenience float seconds
};

class Clock {
public:
	Clock(Clock* parent = nullptr);
	Clock(const Clock& c) = delete;
	~Clock(); // make sure to cleanup the hierarchy

	// resets the clock - everything should be zeros out
	// and the reference hpc variables set 
	// to the current hpc.
	void Reset();

	// used on a root clock
	// calculates elapsed time since last call
	// and calls advance.
	void BeginFrame();

	// advance the clock by the given hpc
	void Advance(u64 elapsedHPC);

	// adds a clock as a child to this clock
	void AddChild(Clock* child);
	void RemoveChild(Clock* child);

	// manipulation
	void SetPaused(bool paused);
	void SetScale(float scale);
	float GetDeltaSeconds() const;

	u64 GetStartHPC() const {return m_startHPC; }
	u64 GetCurrentHPC() const { return total.hpc; }
	u64 GetTotalFrameCount() const { return m_frameCount; }

	float GetCurrentSeconds() const { return (float)total.seconds; }

	static Clock* GetMasterClock();
	static float GetMasterCurrentSeconds();
	static float GetMasterDeltaSeconds();

public:
	TimeUnit_t frame;
	TimeUnit_t total;
	
private:
	// local data we need to track for the clock to work; 
	u64					m_startHPC;      // hpc when the clock was last reset (made)
	u64					m_lastFrameHPC; // hpc during last begin_frame call

	double				m_timeScale = 1.0;
	u64					m_frameCount = 0;        // if you're interested in number of frames your game has processes
	bool				m_paused = false;
	Clock*				m_parent = nullptr;
	std::vector<Clock*> m_children;
};

extern std::unique_ptr<Clock> g_theMasterClock;