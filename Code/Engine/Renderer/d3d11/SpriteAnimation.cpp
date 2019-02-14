#include "Engine/Renderer/d3d11/SpriteAnimation.hpp"
#include "Engine/Renderer/d3d11/SpriteSheet.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/EngineCommon.hpp"

SpriteAnimation::SpriteAnimation(const std::string& name, float durationSeconds, eSpriteAnimMode mode, int startIndex /*= -1*/, int endIndex /*= -1*/)
	: m_durationSeconds(durationSeconds)
	, m_playbackMode(mode) {
	UNUSED(name);
	if(startIndex >= 0){
		AddIndexInRange(startIndex, endIndex);
	}
	else{
		AddIndexInRange(0, m_spriteSheet->GetSpritesCount() - 1);
	}
}

void SpriteAnimation::AddIndex(int idx) {
	m_playbackList.push_back(idx);
}

void SpriteAnimation::AddIndexInRange(int start, int end) {
	for(int i = start; i <= end; ++i){
		m_playbackList.push_back(i);
	}
}

void SpriteAnimation::SetDuration(float durationSeconds) {
	m_durationSeconds = durationSeconds;
}

void SpriteAnimation::JumpToSeconds(float seconds) {
	m_elapsedSeconds = seconds;
}

void SpriteAnimation::JumpToFraction(float fraction) {
	JumpToSeconds(fraction * m_durationSeconds);
}

void SpriteAnimation::ClearPlaybackList() {
	m_playbackList.clear();
}

void SpriteAnimation::Update(float deltaSeconds) {
	if(m_isPlaying){
		m_elapsedSeconds += deltaSeconds;
	}
	if(m_elapsedSeconds >= m_durationSeconds){
		switch (m_playbackMode) {
		case SPRITE_ANIM_MODE_PLAY_TO_END:
			m_elapsedSeconds = m_durationSeconds;
			m_isFinished = true;
			m_isPlaying = false;
			break;
		case SPRITE_ANIM_MODE_LOOPING:
			m_elapsedSeconds -= m_durationSeconds;
			break;
#pragma TODO("SpriteAnimation: add pingpong mode")
		}
	}
}

void SpriteAnimation::Pause() {
	m_isPlaying = false;
}

void SpriteAnimation::Resume() {
	m_isPlaying = true;
}

void SpriteAnimation::Reset() {
	m_isPlaying = true;
	m_isFinished = false;
	m_elapsedSeconds = 0.f;
}

Texture2D* SpriteAnimation::GetTexture() const {
	return m_spriteSheet->GetTexture();
}

AABB2 SpriteAnimation::GetCurrentTexCoords() const {
	int currentIndex = (int)(GetFractionElapsed() * (float)m_playbackList.size());
	return m_spriteSheet->GetUVFromIndex(currentIndex);
}

float SpriteAnimation::GetDurationSeconds() const {
	return m_durationSeconds;
}

float SpriteAnimation::GetSecondsElapsed() const {
	return m_elapsedSeconds;
}

float SpriteAnimation::GetSecondsRemaining() const {
	return m_durationSeconds - m_elapsedSeconds;
}

float SpriteAnimation::GetFractionElapsed() const {
	return m_elapsedSeconds / m_durationSeconds;
}

float SpriteAnimation::GetFractionRemaining() const {
	return 1.f - GetFractionElapsed();
}

bool SpriteAnimation::IsPlaying() const {
	return m_isPlaying;
}

bool SpriteAnimation::IsFinished() const {
	return m_isFinished;
}

eSpriteAnimMode ToSpriteAnimationMode(const std::string& str) {
	if (str == "loop") {
		return SPRITE_ANIM_MODE_LOOPING;
	}
	else if (str == "playthrough") {
		return SPRITE_ANIM_MODE_PLAY_TO_END;
	}
	else if (str == "pingpong") {
		return SPRITE_ANIM_MODE_PINGPONG;
	}
	return SPRITE_ANIM_MODE_PLAY_TO_END;
}
