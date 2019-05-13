#include "Engine/Audio/MIDIPlayer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"

MIDIPlayer::MIDIPlayer() {
	MMRESULT result = ::midiOutOpen(&m_outputHandle, 0, 0, 0, CALLBACK_NULL);
	if (result != MMSYSERR_NOERROR) {
		ERROR_AND_DIE("Failed to open MIDI player device!");
	}
	m_stopWatch = std::make_unique<StopWatch>(g_theMasterClock.get());
//	m_threadHandle = g_theThreadManager.CreateThread(&MIDIPlayer::ThreadWorker, this);
//	ThreadWorker();
}

MIDIPlayer::~MIDIPlayer() {
	m_isRunning = false;
	//g_theThreadManager.Join(m_threadHandle);
	   
	::midiOutClose(m_outputHandle);
}

void MIDIPlayer::PlayNote(u8 note, u8 velocity) {
	velocity = (u8)ClampInt((int)velocity, 0, 127);

	MIDIMessage_t msg(1U, velocity, note);
	::midiOutShortMsg(m_outputHandle, msg.data);
}

void MIDIPlayer::StopNote(u8 note) {
	MIDIMessage_t msg(1U, 0, note);
	::midiOutShortMsg(m_outputHandle, msg.data);
}

void MIDIPlayer::SwitchInstrument(u8 id) {
	u32 data = (u32)id << 8;
	data |= 0xC0;
	::midiOutShortMsg(m_outputHandle, data);
}

void MIDIPlayer::SwitchToDrums() {
	MIDIMessage_t msg(0x0000C9);
	::midiOutShortMsg(m_outputHandle, msg.data);
}

void MIDIPlayer::PlayDrums() {
	MIDIMessage_t msg(0x00402399);
	::midiOutShortMsg(m_outputHandle, msg.data);
}

void MIDIPlayer::ThreadWorker() {
	while (m_isRunning) {
		MIDIMessage_t msg;
// 		msg = MIDIMessage_t(0x000000C0);
// 		::midiOutShortMsg(m_outputHandle, msg.data);

		msg = MIDIMessage_t(1U, 0x7F, 0x3C);
		::midiOutShortMsg(m_outputHandle, msg.data);

// 		msg = MIDIMessage_t(0x007f4090);
// 		::midiOutShortMsg(m_outputHandle, msg.data);
// 
// 		msg = MIDIMessage_t(0x007f4390);
// 		::midiOutShortMsg(m_outputHandle, msg.data);

		::Sleep(300);

		::midiOutShortMsg(m_outputHandle, 0x00003C90);
// 		::midiOutShortMsg(m_outputHandle, 0x00004090);
// 		::midiOutShortMsg(m_outputHandle, 0x00004390);
		::Sleep(300);
	}
}

