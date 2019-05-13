#include "Engine/Core/StringId.hpp"
#include <unordered_map>

static std::unordered_map<std::string, StringId> g_theStringIdTable;

StringId CreateOrGetStringId(const std::string& str) {
	StringId sid;
	if (g_theStringIdTable.find(str) != g_theStringIdTable.end()) {
		sid = g_theStringIdTable[str];
	}
	else {
		sid = (unsigned int)std::hash<std::string>{}(str);
		g_theStringIdTable[str] = sid;
	}
	return sid;
}

std::string GetStringFromSid(const StringId& sid) {
	for (auto it : g_theStringIdTable) {
		if (it.second == sid) {
			return it.first;
		}
	}
	return nullptr;
}
