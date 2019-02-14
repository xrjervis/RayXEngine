#pragma once
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Core/BytePacker.hpp"
#include <string>
#include <memory>
#include <vector>

class BitmapFont;

enum eRCSStatus{
	RCS_HOST,
	RCS_CLIENT,
	RCS_INIT
};

class RemoteCommandService {
public:
	RemoteCommandService();
	~RemoteCommandService();

	void		Update();
	void		Render() const;

	void		Init();
	void		Leave();
	bool		TryJoin(const NetAddress_t& netAddr);
	bool		TryHost(u16 port);
	void		ProcessMessage();
	void		OnReceiveCommand(const std::string& cmdString);
	void		OnReceiveEcho(const std::string& cmdString);
	void		SendCommandMessage(u32 idx, bool isEcho, const std::string& str);
	void		SendCommandMessage(TCPSocket* socket, bool isEcho, const std::string& str);

public:
	TCPSocket*				m_host = nullptr;
	TCPSocket*				m_currentSocket = nullptr;
	std::vector<TCPSocket*> m_connectedSockets;
	eRCSStatus				m_status;
	bool					m_isEchoEnabled = true;

private:
	BitmapFont*				m_bmFont = nullptr;
};

extern std::unique_ptr<RemoteCommandService> g_theRemoteCommandService;