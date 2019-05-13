#include "Engine/Core/Command.hpp"
#include <sstream>

Command::Command(std::string cmdStr) {
	std::istringstream commandStream(cmdStr);
	//Get name by first space
	std::getline(commandStream, m_name, ' ');
	//Get arguments by the end('\n')
	std::string token;
	while (std::getline(commandStream, token, ' ')) {
		m_args.push(token);
	}
}

Command::~Command() {

}

std::string Command::GetName() const{
	return m_name;
}

std::string Command::GetRestArg() {
	std::string leftString;
	while(!m_args.empty()){
		leftString += m_args.front() + ' ';
		m_args.pop();
	}
	return leftString.substr(0, leftString.length() - 1);
}
