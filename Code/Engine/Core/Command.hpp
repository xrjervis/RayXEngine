#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <queue>
#include <map>


class Command {
public:
	Command(std::string cmdStr);
	~Command();

	std::string GetName() const;
	
	template<class T>
	bool GetNextArg(T& out);
	std::string GetRestArg();

public:
	std::string m_name;
	std::queue<std::string> m_args;
};

template<class T>
bool Command::GetNextArg(T& out) {
	T myType;
	if(ParseFromString<T>(myType, m_args.front())){
		out = myType;
		m_args.pop();
		return true;
	}
	return false;
}
