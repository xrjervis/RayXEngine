#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif 
#include <Windows.h>
#include <stdint.h>
#include "Engine/Core/type.hpp"
#include <string>

class LocalTimeData {
public:
	LocalTimeData() {
		::QueryPerformanceFrequency((LARGE_INTEGER*)&m_HPCPerSecond);

		// do the divide now, to not pay the cost later
		m_secondsPerHPC = 1.0 / (double)m_HPCPerSecond;
	}

public:
	u64 m_HPCPerSecond;
	double m_secondsPerHPC;
};

// Gets raw performance counter
u64 GetPerformanceCounter();

// converts a performance count the seconds it represents
double PerformanceCountToSeconds(u64 hpc);

u64 SecondsToPerformanceCount(float seconds);

double GetCurrentSeconds();

double GetElapsedTime(u64 startHPC, u64 endHPC);

std::string GetTimeString(double seconds);

std::string GetTimestamp();