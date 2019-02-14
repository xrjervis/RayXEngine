#pragma once
#include <vector>
#include "Engine/Math/AABB2.hpp"

class SpriteSheet;
class Texture2D;

enum eSpriteAnimMode {
	SPRITE_ANIM_MODE_PLAY_TO_END,
	SPRITE_ANIM_MODE_LOOPING,
	SPRITE_ANIM_MODE_PINGPONG
};

class SpriteAnimation {
public:
	SpriteAnimation(const std::string& name, float durationSeconds, eSpriteAnimMode mode, int startIndex = -1, int endIndex = -1);
	~SpriteAnimation() = default;

	void AddIndex(int idx);
	void AddIndexInRange(int start, int end);
	void SetDuration(float durationSeconds);
	void JumpToSeconds(float seconds);
	void JumpToFraction(float fraction);
	void ClearPlaybackList();

	void Update(float deltaSeconds);
	void Pause();
	void Resume();
	void Reset();

	Texture2D* GetTexture() const;
	AABB2 GetCurrentTexCoords() const; // Based on the current elapsed time
	float GetDurationSeconds() const;
	float GetSecondsElapsed() const;
	float GetSecondsRemaining() const;
	float GetFractionElapsed() const;
	float GetFractionRemaining() const;
	bool IsPlaying() const;
	bool IsFinished() const;

public:
	SpriteSheet* m_spriteSheet = nullptr;
	bool m_isPlaying = true;
	bool m_isFinished = false;
	float m_elapsedSeconds = 0.f;
	float m_durationSeconds = 0.f;
	eSpriteAnimMode m_playbackMode = SPRITE_ANIM_MODE_PLAY_TO_END;
	std::vector<int>	m_playbackList;
};

eSpriteAnimMode ToSpriteAnimationMode(const std::string& str);