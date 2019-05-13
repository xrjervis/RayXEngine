#include "Engine/Net/Net.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/BytePacker.hpp"

std::unique_ptr<Net> g_theNet;

static bool Command_PrintLocalAddress(Command& cmd) {
	if (cmd.m_args.empty()){
		std::vector<std::string> localAddrs;
		NetAddress_t::GetAddressForAllLocal(localAddrs);
		for (auto& it : localAddrs) {
			LogTaggedPrintf("net", "My Address: %s", it.c_str());
		}
		return true;
	}
	else {
		return false;
	}
}

//---------------------------------------------------------------------------------------------

bool Net::StartUp() {
	// pick the version we want;
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	int32_t error = ::WSAStartup(version, &data);
	GUARANTEE_OR_DIE(error == 0, "Net startup error!");

	m_isRunning = true;

	CommandDefinition::Register("net_print_local_address", "[N/A] List my ipv4 address.", Command_PrintLocalAddress);
	return (error == 0);
}

void Net::Shutdown() {
	m_isRunning = false;
	::WSACleanup();
}
