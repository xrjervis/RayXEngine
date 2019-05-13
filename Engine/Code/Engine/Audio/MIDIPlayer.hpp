#pragma once

#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <windows.h>
#pragma comment(lib, "winmm.lib")

constexpr u32 MASK_CHANNEL = 0x00000070;
constexpr u32 MASK_VELOCITY = 0x00FF0000;
constexpr u32 MASK_EVENT = 0x0000FF00;

struct MIDIMessage_t {
	MIDIMessage_t() {
	}

	explicit MIDIMessage_t(u8 channel, u8 velocity, u8 note) {
		SetChannel(channel);
		SetVelocity(velocity);
		SetNote(note);
	}

	explicit MIDIMessage_t(u32 value)
		: data(value) {
	}

	void SetChannel(u32 channel) {
		//ch no.1~16
		channel = (u32)ClampInt((int)channel, 1, 16);
		ClearBits(data, MASK_CHANNEL);
		SetBits(data, channel << 4);
		SetBits(data, 0x00000080);
	}

	void SetVelocity(u32 velocity) {
		velocity = (u32)ClampInt((int)velocity, 0, 127);
		ClearBits(data, MASK_VELOCITY);
		SetBits(data, velocity << 16);
	}

	void SetNote(u32 value) {
		value = (u32)ClampInt((int)value, 0, 255);
		ClearBits(data, MASK_EVENT);
		SetBits(data, value << 8);
	}

	u32 data = 0U;
};

class MIDIPlayer {
public:
	MIDIPlayer();
	~MIDIPlayer();

	void PlayNote(u8 note, u8 velocity = 0x7F);
	void StopNote(u8 note);
	void SwitchInstrument(u8 id);

	void SwitchToDrums();
	void PlayDrums();

private:
	void ThreadWorker();
public:
	HMIDIOUT		m_outputHandle;
	threadHandle	m_threadHandle;
	Uptr<StopWatch> m_stopWatch;
	bool			m_isRunning = true;
};