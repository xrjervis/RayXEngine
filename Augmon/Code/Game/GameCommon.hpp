#pragma once
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <functional>
#include <memory>
#include <array>

enum eActionType : int {
	ACTION_TYPE_SINGLEMONITOR = 0,
	ACTION_TYPE_SCANALLMONITORS,
	ACTION_TYPE_MOTIONDETECTOR,
	NUM_ACTION_TYPES
};

struct Pattern_t {
	Pattern_t(int width, int height, std::vector<Rgba> colors)
		: m_width(width)
		, m_height(height)
		, m_colors(colors) {}
	~Pattern_t() {}

	int GetSize() { return m_width * m_height; }

	std::vector<Rgba> m_colors;
	int m_width;
	int m_height;
};


struct Task_t {
	Task_t(float time, Clock* refClock, const Vector2& pos, std::function<void(float, const Vector2&)> callback) {
		m_stopWatch = std::make_unique<StopWatch>(refClock);
		m_stopWatch->SetTimer(time);
		m_callback = callback;
		m_position = pos;
	}

	void Execute() const {
		m_callback(m_stopWatch->GetNormalizedElapsedTime(), m_position);
	}

	Vector2 m_position;
	Uptr<StopWatch> m_stopWatch;
	std::function<void(float, const Vector2&)> m_callback;
};

extern std::vector<Task_t> g_tasks;
