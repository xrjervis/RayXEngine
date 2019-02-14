#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <ctime>

static LocalTimeData g_localTimeData;

//------------------------------------------------------------------------
// Getting the performance counter

u64 GetPerformanceCounter() {
	u64 hpc;
	::QueryPerformanceCounter((LARGE_INTEGER*)&hpc);
	return hpc;
}

//------------------------------------------------------------------------
// Converting to seconds; 
// relies on gLocalTimeData existing;
double PerformanceCountToSeconds(u64 hpc) {
	return (double)hpc * g_localTimeData.m_secondsPerHPC;
}

u64 SecondsToPerformanceCount(float seconds) {
	return (u64)((float)g_localTimeData.m_HPCPerSecond * seconds);
}

double GetCurrentSeconds() {
	return PerformanceCountToSeconds(GetPerformanceCounter());
}

double GetElapsedTime(u64 startHPC, u64 endHPC) {
	return PerformanceCountToSeconds(endHPC - startHPC);
}

std::string GetTimeString(double seconds) {
	int integer = (int)seconds;
	if (integer > 0) {
		return Stringf("%.2f s", seconds);
	}
	seconds *= 1000.f;
	integer = (int)seconds;
	if (integer > 0) {
		return Stringf("%.2f ms", seconds);
	}
	seconds *= 1000.f;
	return Stringf("%d us", (int)seconds);
}

std::string GetTimestamp() {
	time_t rawtime;
	char buffer[100];
	time(&rawtime);
	struct tm timeinfo;
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
	return std::string(buffer);
}
