#include "Engine/Core/RemoteCommandService.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Camera.hpp"

std::unique_ptr<RemoteCommandService> g_theRemoteCommandService;

constexpr char* RCS_PORT_STR = "29283";
constexpr u16 RCS_PORT = 29283u;

static void EchoResponse(const std::string& message){
	g_theRemoteCommandService->SendCommandMessage(g_theRemoteCommandService->m_currentSocket, true, message);
}

static bool Command_rc(Command& cmd){
	Command copy = cmd;
	int idx = -1;
	copy.GetNextArg<int>(idx);
	std::string message = copy.GetRestArg();

	// rc help
	if(message.empty()){
		message = cmd.GetRestArg();
		g_theRemoteCommandService->SendCommandMessage(0u, false, message);
		return true;
	}
	else if (!message.empty() && idx < 0){
		message = cmd.GetRestArg();
		g_theRemoteCommandService->SendCommandMessage(0u, false, message);
		return true;
	}
	else if (idx >= 0 && !message.empty()){
		g_theRemoteCommandService->SendCommandMessage(idx, false, message);
		return true;
	}
	else{
		return false;
	}
}

static bool Command_rcb(Command& cmd){
	std::string message = cmd.GetRestArg();
	for (TCPSocket* socket : g_theRemoteCommandService->m_connectedSockets) {
		if (!socket->IsClosed()) {
			g_theRemoteCommandService->SendCommandMessage(socket, false, message);
		}
	}
	return true;
}

static bool Command_rca(Command& cmd){
	std::string message = cmd.GetRestArg();
	for (TCPSocket* socket : g_theRemoteCommandService->m_connectedSockets) {
		if (!socket->IsClosed()) {
			g_theRemoteCommandService->SendCommandMessage(socket, false, message);
		}
	}
	Command newCmd(message);
	g_theConsole->RunCommand(newCmd);
	return true;
}

static bool Command_rcjoin(Command& cmd){
	if (cmd.m_args.size() == 1) {
		std::string addr;
		cmd.GetNextArg<std::string>(addr);
		g_theRemoteCommandService->Leave();
		NetAddress_t netAddr(addr);
		if(g_theRemoteCommandService->TryJoin(netAddr)){
			g_theRemoteCommandService->m_status = RCS_CLIENT;
		}
		else{
			g_theRemoteCommandService->m_status = RCS_INIT;
		}
		return true;
	}
	else {
		return false;
	}
}

static bool Command_rchost(Command& cmd){
	if (cmd.m_args.size() == 1) {
		std::string port;
		cmd.GetNextArg<std::string>(port);
		g_theRemoteCommandService->Leave();
		if(g_theRemoteCommandService->TryHost((u16)std::stoul(port))){
			g_theRemoteCommandService->m_status = RCS_HOST;
		}
		else {
			g_theRemoteCommandService->m_status = RCS_INIT;
		}
		return true;
	}
	else {
		return false;
	}
}

RemoteCommandService::RemoteCommandService() {
	CommandDefinition::Register("rc", "[(int), string] Send a command to the connection associated with the index. If no index is given, will default to 0", Command_rc);
	CommandDefinition::Register("rcb", "[string] Send a broadcast command.", Command_rcb);
	CommandDefinition::Register("rca", "[string] Send a command to all connections.", Command_rca);
	CommandDefinition::Register("rcs_join", "[string] Leave current service, and attempt to join address instead of local.", Command_rcjoin);
	CommandDefinition::Register("rcs_host", "[u16] Leave current service, and attempt to host on the given port.", Command_rchost);

	Init();
}

RemoteCommandService::~RemoteCommandService() {
	if(m_status == RCS_HOST){
		m_host->Close();
		SAFE_DELETE(m_host);
	}
	for(TCPSocket* it : m_connectedSockets){
		it->Close();
		delete it;
	}
}

void RemoteCommandService::Update() {
	if(m_status == RCS_INIT){
		Init();
	}
	else{
		// delete closed connections
		for(size_t i = 0; i < m_connectedSockets.size(); ++i){
			if(m_connectedSockets[i]->IsClosed()){
				m_connectedSockets.erase(std::remove(m_connectedSockets.begin(), m_connectedSockets.end(), m_connectedSockets[i]), m_connectedSockets.end());
				i--;
			}
		}
		// host accept connection
		if(m_status == RCS_HOST){
			TCPSocket* client = m_host->Accept();
			if (client) {
				m_connectedSockets.push_back(client);
			}

		}

		ProcessMessage();
	}
	
}

void RemoteCommandService::Render() const {
// 	static float startX = Window::Get(0)->GetOrtho().maxs.x - 500.f;
// 	static float startY = Window::Get(0)->GetOrtho().maxs.y - 30.f;
	char* status = "";

	switch(m_status) {
	case RCS_HOST: status = "Host"; break;
	case RCS_CLIENT: status = "Client"; break;
	case RCS_INIT: status = "Init"; break;
	}

// 	g_theRenderer->DrawText(m_bmFont, Vector2(startX, startY), Vector2::ZERO, Stringf("Remote Command Service [%s]", status));
// 	g_theRenderer->DrawText(m_bmFont, Vector2(startX, startY - 48.f), Vector2::ZERO, Stringf("Connections: ", status), 24.f);

	if(m_status == RCS_HOST){
//		g_theRenderer->DrawText(m_bmFont, Vector2(startX, startY - 24.f), Vector2::ZERO, Stringf("Host at [%s]", m_host->m_address.ToString().c_str()), 24.f);

	}
	else if(m_status == RCS_CLIENT){
//		g_theRenderer->DrawText(m_bmFont, Vector2(startX, startY - 24.f), Vector2::ZERO, Stringf("Connected to [%s]", m_currentSocket->m_address.ToString().c_str()), 24.f);

	}

	for(size_t i = 0; i < m_connectedSockets.size(); ++i){
//		g_theRenderer->DrawText(m_bmFont, Vector2(startX, (startY - 72.f) - 24.f * i), Vector2::ZERO, Stringf("[%u] %s", i, m_connectedSockets[i]->m_address.ToString().c_str()), 24.f);
	}
}

void RemoteCommandService::Init() {
	std::vector<std::string> localAddrs;
	NetAddress_t::GetAddressForAllLocal(localAddrs);
	std::string ipAddress = localAddrs[localAddrs.size() - 1];
	NetAddress_t netAddr(ipAddress + ":" + RCS_PORT_STR);
	if (TryJoin(netAddr)) {
		m_status = RCS_CLIENT;
	}
	else {
		if (TryHost(RCS_PORT)) {
			m_status = RCS_HOST;
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			m_status = RCS_INIT;
		}
	}
}

void RemoteCommandService::Leave() {
	if(m_status == RCS_CLIENT){
		m_currentSocket->Close();
		SAFE_DELETE(m_currentSocket);
	}
	else if (m_status == RCS_HOST){
		m_host->Close();
		SAFE_DELETE(m_host);
		for(TCPSocket* socket : m_connectedSockets){
			socket->Close();
		}
	}
	m_connectedSockets.clear();
}

bool RemoteCommandService::TryJoin(const NetAddress_t& netAddr) {
	TCPSocket* socket = new TCPSocket();
	if (socket->Connect(netAddr)) {
		socket->m_address = netAddr;
		m_connectedSockets.push_back(socket);
		return true;
	}
	else{
		delete socket;
		LogErrorf("Try join at [%s] failed.", netAddr.ToString().c_str());
		return false;
	}
}

bool RemoteCommandService::TryHost(u16 port) {
	m_host = new TCPSocket();
	if (m_host->Listen(port, 16u)) {
		return true;
	}
	else{
		LogErrorf("Try host at port:%u failed", port);
		return false;
	}
}

void RemoteCommandService::ProcessMessage() {
	// Process messages
	for (TCPSocket* socket : m_connectedSockets) {
		m_currentSocket = socket;

		// Get total message length
		u16 messageLength = 0;
		size_t received = m_currentSocket->Receive(&messageLength, 2);

		// Must Have!!!
		FromEndianness(2, &messageLength, BIG_ENDIAN);
		if (m_status == RCS_CLIENT) {
			// lost connection to host, then switch to host my self
			if (m_currentSocket->IsClosed()) {
				m_connectedSockets.clear();
				m_status = RCS_INIT;
				return;
			}
		}

		//FromEndianness(2, &messageLength, BIG_ENDIAN);
		if (received > 0) {
			// Get isEcho bool, string size, string data ( unrelated to endianness)
			u8 buffer[1024];
			while(m_currentSocket->Receive(buffer, messageLength) == 0){

			}

			if (received > 0) {
				// Create a byte packer for the message
				BytePacker packer(messageLength, buffer, BIG_ENDIAN);
				packer.SetReadableByteCount(messageLength);
				bool isEcho;
				packer.ReadBytes(&isEcho, 1);
				std::string message;
				packer.ReadString(message);
				if (isEcho) {
					OnReceiveEcho(message);
				}
				else {
					OnReceiveCommand(message);
				}
			}
		}
	}
}

void RemoteCommandService::OnReceiveCommand(const std::string& cmdString) {
	Command cmd(cmdString);
	g_theConsole->Hook(EchoResponse);
	g_theConsole->RunCommand(cmd);
	g_theConsole->UnHook(EchoResponse);
}

void RemoteCommandService::OnReceiveEcho(const std::string& cmdString) {
	ConsolePrintf(Rgba::YELLOW, "[%s] %s", m_currentSocket->m_address.ToString().c_str(), cmdString.c_str());
}

void RemoteCommandService::SendCommandMessage(u32 idx, bool isEcho, const std::string& str) {
	if(idx >= m_connectedSockets.size()){
		return;
	}
	TCPSocket* socket = m_connectedSockets[idx];
	if (socket == nullptr) {
		return;
	}
	SendCommandMessage(socket, isEcho, str);
}

void RemoteCommandService::SendCommandMessage(TCPSocket* socket, bool isEcho, const std::string& str) {
	if(socket == nullptr){
		return;
	}
	BytePacker message(BIG_ENDIAN);

	message.WriteBytes(&isEcho, 1);
	message.WriteString(str);

	size_t len = message.GetWrittenByteCount();
	u16 uslen = (u16)len;

	ToEndianness(2, &uslen, BIG_ENDIAN);
	if (socket->Send(&uslen, 2) == 0u){
		return;
	}
	if (socket->Send(message.GetBuffer(), len) == 0u) {
		return;
	}
}