#pragma once
#include "Engine/Core/type.hpp"
#include <string>
typedef u32 StringId;

extern StringId CreateOrGetStringId(const std::string& str);
extern std::string GetStringFromSid(const StringId& sid);