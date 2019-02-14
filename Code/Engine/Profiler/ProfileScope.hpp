#pragma once
#include "Engine/Core/type.hpp"
#include <string>

class LogProfileScope {
public:
	LogProfileScope(const std::string& tag);
	~LogProfileScope();

private:
	std::string m_tag;
	u64 m_startHPC;
};

#define PROFILE_LOG_SCOPE(s) LogProfileScope __timer_ ##__LINE__ ## (s)
#define PROFILE_LOG_SCOPE_FUNCTION() LogProfileScope __timer_ ##__LINE__ ## (__FUNCTION__)

class ProfileScope {
public:
	ProfileScope(const std::string& tag);
	~ProfileScope();

private:
	std::string m_tag;
};

#define PROFILE_SCOPE(s) ProfileScope __timer_ ##__LINE__ ## (s)
#define PROFILE_SCOPE_FUNTION() ProfileScope __timer_ ##__LINE__ ## (__FUNCTION__)
