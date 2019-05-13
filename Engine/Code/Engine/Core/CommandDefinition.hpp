#pragma once
#include "Engine/Core/Command.hpp"
#include <string>
#include <map>

typedef bool(*command_cb)(Command& cmd);

class CommandDefinition {
	friend class DevConsole;
public:
	CommandDefinition() = default;
	CommandDefinition(std::string, std::string, command_cb);

	static void Register(std::string cmdName,std::string description, command_cb cb);
	static CommandDefinition* GetDefinition(std::string cmdName);

public:
	static std::map<std::string, CommandDefinition*> s_commandDefintions;

private:
	std::string m_name;
	std::string m_description;
	command_cb m_callback;
};