#include "Game/Board.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <sstream>


//----------------------------------------------------------------------------------
// RS2 prerequisites
rs2::pipeline			g_pipe;
rs2::config				g_cfg;
rs2::pipeline_profile	g_profile;
rs2::align				g_alignObject(RS2_STREAM_COLOR);
rs2::colorizer			g_colorMap;

// Declare filters
rs2::decimation_filter dec_filter;  // Decimation - reduces depth frame density
rs2::spatial_filter spat_filter;    // Spatial    - edge-preserving spatial smoothing
rs2::temporal_filter temp_filter;   // Temporal   - reduces temporal noise
rs2::hole_filling_filter hole_filter;

// Declare disparity transform from depth to disparity and vice versa
rs2::disparity_transform depth_to_disparity(true);
rs2::disparity_transform disparity_to_depth(false);

// Color Range in HSV space
// In OpenCV H has values from 0 to 180, S and V from 0 to 255. 
// The red color has the hue values approximately in the range of 0 to 10 and 160 to 180.
// So normally when need to get red color, we invert the image, and look for cyan color range
cv::Scalar g_blackRange[2]	= { cv::Scalar(0, 0, 0 ), cv::Scalar(179, 255, 50)};
cv::Scalar g_whiteRange[2]	= { cv::Scalar(0, 0, 100), cv::Scalar(179, 80, 255) };
cv::Scalar g_cyanRange[2]	= { cv::Scalar(80, 70, 100), cv::Scalar(100, 255, 255) };
cv::Scalar g_greenRange[2]	= { cv::Scalar(40, 70, 100), cv::Scalar(80, 255, 255) };
cv::Scalar g_blueRange[2]	= { cv::Scalar(100, 70, 100), cv::Scalar(130, 255, 255) };
cv::Scalar g_yellowRange[2] = { cv::Scalar(10, 70, 100), cv::Scalar(40, 255, 255) };

Board::Board() {
	//------------------------------------------------------------------------
	// Init Intel RealSense Camera
	g_cfg.enable_stream(RS2_STREAM_INFRARED, 1280, 720, RS2_FORMAT_Y8, 30);
	g_cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
	g_cfg.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_BGR8, 30);
	g_profile = g_pipe.start(g_cfg);
	m_depthScale = GetDepthScale(g_profile.get_device());

	cv::namedWindow("test", cv::WINDOW_AUTOSIZE);
	m_threadHandle = g_theThreadManager.CreateThread(&Board::ThreadWorker, this);
}

Board::~Board() {
	m_isRunning = false;
	g_theThreadManager.Join(m_threadHandle);
}

float Board::GetDepthScale(const rs2::device& device) {
	for (rs2::sensor& sensor : device.query_sensors()) {
		if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>()) {
			return dpt.get_depth_scale();
		}
	}
	// cannot find a depth sensor
	return 1.0f;
}

void Board::ExtractColorInHSVFromRawImage(cv::Scalar* colorRange, cv::Mat& src, cv::Mat& dst, bool isRed /*= false*/) {
	if (isRed) {
		src = ~src;
	}
	cv::Mat maskHSV, hsvImage;
	cv::cvtColor(src, hsvImage, cv::COLOR_BGR2HSV);
	cv::inRange(hsvImage, colorRange[0], colorRange[1], maskHSV);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));
	cv::morphologyEx(maskHSV, maskHSV, cv::MORPH_OPEN, element);
	element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
	cv::morphologyEx(maskHSV, dst, cv::MORPH_CLOSE, element);
}

void Board::GetObjectPositionsFromBinaryImage(const cv::Mat& src, std::vector<Vector2>& positions) const {
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(src, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	for (size_t i = 0; i < contours.size(); ++i) {
		if (contours[i].size() < 3) {
			continue;
		}
		std::vector<std::vector<cv::Point>> contoursPoly(contours.size());
		cv::approxPolyDP(contours[i], contoursPoly[i], 9, true);

		cv::Moments M = cv::moments(contours[i]);
		if (M.m00 == 0.f) {
			continue;
		}
		int cX = int(M.m10 / M.m00);
		int cY = int(M.m01 / M.m00);
		positions.emplace_back(cX, cY);
	}
}

void Board::ThreadWorker() {
	while (m_isRunning) {
		PROFILE_LOG_SCOPE("Manipulate Camera Data");

		rs2::frameset frameset = g_pipe.wait_for_frames();
		// Get processed aligned frame
		auto alignedFrames = g_alignObject.process(frameset);

		// Trying to get both other and aligned depth frames
		rs2::video_frame irFrame = alignedFrames.get_infrared_frame();
		rs2::video_frame colorFrame = alignedFrames.get_color_frame();
		rs2::depth_frame depthFrame = alignedFrames.get_depth_frame();

		// Apply post-processing filters to depth image
		//depthFrame = dec_filter.process(depthFrame);
		depthFrame = depth_to_disparity.process(depthFrame);
		depthFrame = spat_filter.process(depthFrame);
		depthFrame = temp_filter.process(depthFrame);
		depthFrame = disparity_to_depth.process(depthFrame);
		depthFrame = hole_filter.process(depthFrame);

		const int frameWidth = depthFrame.get_width();
		const int frameHeight = depthFrame.get_height();

		cv::Mat colorImage; // Camera's color image
		colorImage = cv::Mat(cv::Size(frameWidth, frameHeight), CV_8UC3, (void*)colorFrame.get_data(), cv::Mat::AUTO_STEP);

		const uint16_t* depthValueArray = reinterpret_cast<const uint16_t*>(depthFrame.get_data());

		//-----------------------------------------------------------------------------
		// If not ready, doing initial alignment
		if (!m_isReady) {
			m_counter++;
			if (m_counter < 10) {
				continue;
			}
			cv::Mat alignedBinaryImage;
			ExtractColorInHSVFromRawImage(g_cyanRange, colorImage, alignedBinaryImage, true);
			//debug
			//cv::imshow("test", alignedBinaryImage);
			std::vector<Vector2> positions;
			GetObjectPositionsFromBinaryImage(alignedBinaryImage, positions);
			if (positions.size() == 4) {
				Vector2 boardCenter(BOARD_WIDTH_IN_CAMERA * .5f, BOARD_HEIGHT_IN_CAMERA * .5f);
				for (int i = 0; i < 4; ++i) {
					Vector2 pos(positions[i].x, positions[i].y);
					if (pos.x < boardCenter.x && pos.y < boardCenter.y) {
						m_tlCorner = pos;
					}
					else if (pos.x < boardCenter.x && pos.y > boardCenter.y) {
						m_blCorner = pos;
					}
					else if (pos.x > boardCenter.x && pos.y < boardCenter.y) {
						m_trCorner = pos;
					}
					else if (pos.x > boardCenter.x && pos.y > boardCenter.y) {
						m_brCorner = pos;
					}
				}
				m_tlCorner.y = BOARD_HEIGHT_IN_CAMERA - m_tlCorner.y;
				m_blCorner.y = BOARD_HEIGHT_IN_CAMERA - m_blCorner.y;
				m_trCorner.y = BOARD_HEIGHT_IN_CAMERA - m_trCorner.y;
				m_brCorner.y = BOARD_HEIGHT_IN_CAMERA - m_brCorner.y;

				m_isReady = true;
			}
			cv::imshow("test", colorImage);
		}
		else {
			PROFILE_LOG_SCOPE("Gridlization");
			cv::Mat hsvImage;
			cv::Mat hsvImageInverse;
			cv::Mat colorImageInverse = ~colorImage;
			//cv::Mat debugImage = cv::Mat::zeros(frameWidth, frameHeight, CV_8U);
			cv::cvtColor(colorImage, hsvImage, cv::COLOR_BGR2HSV);
			cv::cvtColor(colorImageInverse, hsvImageInverse, cv::COLOR_BGR2HSV);

			BoardData_t* data = m_boardData.BeginWrite();
			std::fill(data->blackFractions.begin(), data->blackFractions.end(), 0.f);
			std::fill(data->redFractions.begin(), data->redFractions.end(), 0.f);
			std::fill(data->greenFractions.begin(), data->greenFractions.end(), 0.f);
			std::fill(data->blueFractions.begin(), data->blueFractions.end(), 0.f);
			std::fill(data->yellowFractions.begin(), data->yellowFractions.end(), 0.f);
			std::fill(data->whiteFractions.begin(), data->whiteFractions.end(), 0.f);
			std::fill(data->depthValues.begin(), data->depthValues.end(), 0.f);

			cv::circle(colorImage, cv::Point(m_blCorner.x, BOARD_HEIGHT_IN_CAMERA - m_blCorner.y), 2, cv::Scalar(0, 255, 0));
			cv::circle(colorImage, cv::Point(m_brCorner.x, BOARD_HEIGHT_IN_CAMERA - m_brCorner.y), 2, cv::Scalar(0, 255, 0));
			cv::circle(colorImage, cv::Point(m_tlCorner.x, BOARD_HEIGHT_IN_CAMERA - m_tlCorner.y), 2, cv::Scalar(0, 255, 0));
			cv::circle(colorImage, cv::Point(m_trCorner.x, BOARD_HEIGHT_IN_CAMERA - m_trCorner.y), 2, cv::Scalar(0, 255, 0));


			for (int gridY = 0; gridY < GRID_COUNT_Y; ++gridY) {
				for (int gridX = 0; gridX < GRID_COUNT_X; ++gridX) {
					float widthFraction = (float)gridX / (float)GRID_COUNT_X;
					float heightFraction = (float)gridY / (float)GRID_COUNT_Y;

					Vector2 leftPoint = m_blCorner + RangeMapFloat((float)gridY, 0.f, (float)(GRID_COUNT_Y - 1), 0.f, (float)GRID_COUNT_Y) * ((m_tlCorner - m_blCorner) / (float)GRID_COUNT_Y);
					Vector2 rightPoint = m_brCorner + RangeMapFloat((float)gridY, 0.f, (float)(GRID_COUNT_Y - 1), 0.f, (float)GRID_COUNT_Y) * ((m_trCorner - m_brCorner) / (float)GRID_COUNT_Y);
					Vector2 topPoint = m_tlCorner + RangeMapFloat((float)gridX, 0.f, (float)(GRID_COUNT_X - 1), 0.f, (float)GRID_COUNT_X) * ((m_trCorner - m_tlCorner) / (float)GRID_COUNT_X);
					Vector2 bottomPoint = m_blCorner + RangeMapFloat((float)gridX, 0.f, (float)(GRID_COUNT_X - 1), 0.f, (float)GRID_COUNT_X) * ((m_brCorner - m_blCorner) / (float)GRID_COUNT_X);

					float gridActualWidth = (rightPoint - leftPoint).GetLength() / GRID_COUNT_X;
					float gridActualHeight = (topPoint - bottomPoint).GetLength() / GRID_COUNT_Y;
					float maskWidth = 0.6f * gridActualWidth;
					float maskHeight = 0.6f * gridActualHeight;

					Vector2 gridCenter;
					bool result = DoLineSegmentsIntersect(leftPoint, rightPoint, bottomPoint, topPoint, gridCenter);
					if (!result) {
						ERROR_AND_DIE("Something wrong!");
					}

					Vector2 maskBLCorner(gridCenter.x - 0.25f * gridActualWidth, frameHeight - (gridCenter.y + 0.25f * gridActualHeight));

					cv::rectangle(colorImage, cv::Rect(RoundToNearestInt(maskBLCorner.x), RoundToNearestInt(maskBLCorner.y), RoundToNearestInt(maskWidth), RoundToNearestInt(maskHeight)), cv::Scalar(0, 0, 100), -1);

					cv::Mat gridImage = hsvImage(cv::Rect(RoundToNearestInt(maskBLCorner.x), RoundToNearestInt(maskBLCorner.y), RoundToNearestInt(maskWidth), RoundToNearestInt(maskHeight)));
					cv::Mat gridImageInverse = hsvImageInverse(cv::Rect(RoundToNearestInt(maskBLCorner.x), RoundToNearestInt(maskBLCorner.y), RoundToNearestInt(maskWidth), RoundToNearestInt(maskHeight)));

					// Calculate Histogram for H and S, respectively
					std::vector<cv::Mat> hsvValues;
					std::vector<cv::Mat> hsvValuesInverse;

					cv::split(gridImage, hsvValues);
					cv::split(gridImageInverse, hsvValuesInverse);


					int totalPixelsInGrid = gridImage.rows * gridImage.cols;
					int blackPixelsInGrid = 0;
					int whitePixelsInGrid = 0;
					int redPixelsInGrid = 0;
					int greenPixelsInGrid = 0;
					int bluePixelsInGrid = 0;
					int yellowPixelsInGrid = 0;

					float totalDepthValuesInGrid = 0.f;

					int pixelBL_X = RoundToNearestInt(maskBLCorner.x);
					int pixelBL_Y = RoundToNearestInt(maskBLCorner.y);

					for (int row = 0; row < gridImage.rows; ++row) {
						for (int col = 0; col < gridImage.cols; ++col) {
							uchar h_value = hsvValues[0].at<uchar>(row, col);  //[0, 179]  1011,0011
							uchar s_value = hsvValues[1].at<uchar>(row, col);
							uchar v_value = hsvValues[2].at<uchar>(row, col);
							uchar h_value_inverse = hsvValuesInverse[0].at<uchar>(row, col);
							if (v_value <= g_blackRange[1][2]) {
								blackPixelsInGrid++;
							}
							else if (g_whiteRange[0][1] <= s_value && s_value <= g_whiteRange[1][1]) {
								whitePixelsInGrid++;
							}
							else if (g_cyanRange[0][0] <= h_value_inverse && h_value_inverse <= g_cyanRange[1][0]) {
								redPixelsInGrid++;
							}
							else if (g_greenRange[0][0] <= h_value && h_value <= g_greenRange[1][0]) {
								greenPixelsInGrid++;
							}
							else if (g_blueRange[0][0] <= h_value && h_value <= g_blueRange[1][0]) {
								bluePixelsInGrid++;
							}
							else if (g_yellowRange[0][0] <= h_value && h_value <= g_yellowRange[1][0]) {
								yellowPixelsInGrid++;
							}

							int currentPixelX = pixelBL_X + col;
							int currentPixelY = pixelBL_Y + row;
							int pixelIndex = currentPixelY * frameWidth + currentPixelX;
							uint16_t depthValue = depthValueArray[pixelIndex];
							totalDepthValuesInGrid += (float)depthValue * 0.001f;
						}
					}

					int gridIndex = gridY * GRID_COUNT_X + gridX;
					data->blackFractions[gridIndex] = (float)blackPixelsInGrid / (float)totalPixelsInGrid;
					data->whiteFractions[gridIndex] = (float)whitePixelsInGrid / (float)totalPixelsInGrid;
					data->redFractions[gridIndex] = (float)redPixelsInGrid / (float)totalPixelsInGrid;
					data->greenFractions[gridIndex] = (float)greenPixelsInGrid / (float)totalPixelsInGrid;
					data->blueFractions[gridIndex] = (float)bluePixelsInGrid / (float)totalPixelsInGrid;
					data->yellowFractions[gridIndex] = (float)yellowPixelsInGrid / (float)totalPixelsInGrid;


					//depth value
// 					int pixelX = gridX * gridActualWidth + gridActualWidth / 2;
// 					int pixelY = gridY * gridActualHeight + gridActualHeight / 2;
// 					int index = pixelY * frameWidth + pixelX;
//  				int depthValue = depthValueArray[index];

					data->depthValues[gridIndex] = totalDepthValuesInGrid / (float)totalPixelsInGrid;
				}
			}
			m_boardData.EndWrite();
			cv::imshow("test", colorImage);
		}
	}
}
