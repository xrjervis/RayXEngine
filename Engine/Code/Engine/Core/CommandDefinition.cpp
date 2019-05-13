#include "Engine/Core/CommandDefinition.hpp"

std::map<std::string, CommandDefinition*> CommandDefinition::s_commandDefintions;

CommandDefinition::CommandDefinition(std::string name, std::string description, command_cb cb) 
	: m_name(name)
	, m_description(description)
	, m_callback(cb) {
}

void CommandDefinition::Register(std::string cmdName, std::string description, command_cb cb) {
	CommandDefinition* cmdDef = new CommandDefinition(cmdName, description, cb);
	s_commandDefintions.insert({ cmdName, cmdDef });
}

CommandDefinition* CommandDefinition::GetDefinition(std::string cmdName) {
	return s_commandDefintions[cmdName];
}
