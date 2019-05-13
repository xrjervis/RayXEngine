#pragma once
#include "opencv2/opencv.hpp"
#include "librealsense2/rs.hpp"
#include "opencv2/core/directx.hpp"
#include "opencv2/core/ocl.hpp"
#include "Engine/Core/TripleBuffer.hpp"
#include "Engine/Core/Thread.hpp"
#include <memory>
#include <vector>
#include <array>
#include <future>

#define ACTIVATE_BOARD