#pragma once
#include "Game/Augmon.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Core/TripleBuffer.hpp"
#include <mutex>
#include <vector>

constexpr int GRID_COUNT_X = 42;
constexpr int GRID_COUNT_Y = 23;
constexpr int GRID_COUNT_TOTAL = GRID_COUNT_X * GRID_COUNT_Y;
constexpr float WIDTH_IN_BOARD_WINDOW = 1920.f;
constexpr float HEIGHT_IN_BOARD_WINDOW = 1080.f;
constexpr float GRID_WIDTH_IN_BOARD_WINDOW = WIDTH_IN_BOARD_WINDOW / (float)GRID_COUNT_X;
constexpr float GRID_HEIGHT_IN_BOARD_WINDOW = HEIGHT_IN_BOARD_WINDOW / (float)GRID_COUNT_Y;
constexpr float BOARD_WIDTH_IN_CAMERA = 1280.f;
constexpr float BOARD_HEIGHT_IN_CAMERA = 720.f;


// Normalized [0.f, 1.f] for each grid
struct BoardData_t {
	Rgba GetColorAtGridCoords(int x, int y) {
		int gridIndex = GRID_COUNT_X * y + x;
		return GetColorAtGridIndex(gridIndex);
	}
	Rgba GetColorAtGridIndex(int index) {
		Rgba color;
		if (blackFractions[index] > 0.8f) {
			color = Rgba::BLACK;
		}
		else if (whiteFractions[index] > 0.8f) {
			color = Rgba::WHITE;
		}
		else if (redFractions[index] > 0.8f) {
			color = Rgba::RED;
		}
		else if (greenFractions[index] > 0.8f) {
			color = Rgba::GREEN;
		}
		else if (blueFractions[index] > 0.8f) {
			color = Rgba::BLUE;
		}
		else if (yellowFractions[index] > 0.8f) {
			color = Rgba::YELLOW;
		}
		return color;
	}

	std::array<float, GRID_COUNT_TOTAL> blackFractions;
	std::array<float, GRID_COUNT_TOTAL> whiteFractions;
	std::array<float, GRID_COUNT_TOTAL> redFractions;
	std::array<float, GRID_COUNT_TOTAL> greenFractions;
	std::array<float, GRID_COUNT_TOTAL> blueFractions;
	std::array<float, GRID_COUNT_TOTAL> yellowFractions;
	std::array<float, GRID_COUNT_TOTAL> depthValues;
};

class SpriteSheet;

class Board {
public:
	Board();
	~Board();

	void ThreadWorker();

private:
	float GetDepthScale(const rs2::device& device);
	// If the color is red, flip over the entire source image
	// and then go for color cyan. 
	void ExtractColorInHSVFromRawImage(cv::Scalar* colorRange, cv::Mat& src, cv::Mat& dst, bool isRed = false);
	void GetObjectPositionsFromBinaryImage(const cv::Mat& src, std::vector<Vector2>& positions) const;

public:
	TripleBuffer<BoardData_t>			m_boardData;

	int									m_counter = 0;

	bool								m_isReady = false;
	std::mutex							m_mutex;
	Vector2								m_blCorner;
	Vector2								m_brCorner;
	Vector2								m_tlCorner;
	Vector2								m_trCorner;

private:
	threadHandle						m_threadHandle;
	float								m_depthScale = 1.f;
	bool								m_isRunning = true;
};