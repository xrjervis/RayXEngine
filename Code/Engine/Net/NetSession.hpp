#pragma once
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Net/NetConnectionInfo.hpp"
#include "Engine/Net/NetObjectSystem.hpp"
#include "Engine/Core/Thread.hpp"

class NetConnection;
class StopWatch;

constexpr u8 INVALID_CONN_INDEX = 0xFF;
constexpr size_t MAX_CONNECTIONS = 16;
constexpr float DEFAULT_CONNECTION_TIMEOUT = 10.f;
constexpr float MAX_NET_TIME_DILATION = 0.1f;

enum eSessionState{
	SESSION_DISCONNECTED = 0,
	SESSION_BOUND,
	SESSION_CONNECTING,
	SESSION_JOINING,
	SESSION_READY
};

enum eSessionError{
	SESSION_OK,
	SESSION_ERROR_BIND_SOCKET_FAILED,
	SESSION_ERROR_USER_DISCONNECT,
	SESSION_ERROR_JOIN_DENIED,
	SESSION_ERROR_JOIN_DENIED_NOT_HOST,
	SESSION_ERROR_JOIN_DENIED_CLOSED,
	SESSION_ERROR_JOIN_DENIED_FULL,
	SESSION_ERROR_JOIN_TIMEOUT
};

class NetSession {
public:
	NetSession();
	~NetSession();

	void						RegisterCoreMessages();
	void						RegisterGameMessages();
	void						FinalizeNetMessageIndex();
	void						RegisterMessageDefinition(const std::string& name, NetMessage_cb cb, eNetMessageOption options = NETMSGOPTION_NONE);
	void						RegisterMessageDefinition(u8 idx, const std::string& name, NetMessage_cb cb, eNetMessageOption options = NETMSGOPTION_NONE);

	NetMessageDefinition_t*		GetMessageDefinitionByName(const std::string& name);
	NetMessageDefinition_t*		GetMessageDefinitionByIndex(u8 idx);
	NetConnection*				GetConnection(u8 idx);
	NetConnection*				GetConnection(const NetAddress_t& addr);
	void						CloseAllConnections();
	void						ProcessIncomingPackets();
	void						ProcessOutgoingPackets();
	void						SendDirectMessage(const NetAddress_t& addr, const NetMessage& msg);
	void						SendDirectMessage(u8 connectionIdx, const NetMessage& msg);
	void						ProcessDirectMessage(NetPacket* packet, PacketHeader_t* header);

	void						SetSimLoss(float lossChance);
	void						SetSimLatency(float min, float max);
	void						SetHeartbeatRate(float rate);

	void						Host(u16 port, u16 portRange = DEFAULT_PORT_RANGE);
	void						Join(const NetConnectionInfo_t& hostInfo);
	void						Update();

	void						SetError(eSessionError error, const std::string& errorStr);
	void						ClearError();
	eSessionError				GetLastError(std::string& outStr);

	void						Render() const;
	inline bool					IsHost() const { return m_myConnection ? m_myConnection == m_hostConnection : false; }
	bool						IsConnectionFull() const;
	size_t						GetAvailableConnectionSlot() const;
	bool						DoesNetAddressExist(const NetAddress_t& addr) const;

	NetConnection*				CreateConnection(const NetConnectionInfo_t& info);
	void						DestroyConnection(NetConnection* conn);
	void						BindConnection(u8 idx, NetConnection* conn);
	void						Disconnect(NetConnection* conn);

	float						GetNetTimeInSeconds() const;

	NetObjectSystem*			GetNetObjectSystem() { return &m_netObjectSystem; }
	void						SendToAllOthers(const NetMessage& msg, eConnectionState connState);

public:
	NetObjectSystem									m_netObjectSystem;
	std::function<void(NetConnection*, const std::string&)>	m_onJoinCallback;
	std::function<void(NetConnection*)>				m_onLeaveCallback;
	std::string										m_name = "default";
	std::string										m_hostAddress = "10.8.140.44:10084";

	UDPSocket*										m_socket;
	NetConnection*									m_connections[MAX_CONNECTIONS];
	NetConnection*									m_myConnection = nullptr;
	u8												m_connectionIndex = INVALID_CONN_INDEX;
	NetConnection*									m_hostConnection = nullptr;
	float											m_joinRequestTimer = 0.f;
	float											m_sessionConnectingTimeoutTimer = 0.f;

	std::map<std::string, NetMessageDefinition_t*>	m_netMessageDefinitions;
	float											m_lossChance = 0.f;		//[0.f, 1.f]
	float											m_minLatency = 0.f;
	float											m_maxLatency = 0.f;
	float											m_sendRate = 0.05f; // in seconds 50ms

	bool											m_fixedNetmessageIndexArray[255] = { false };
	eSessionState									m_state = SESSION_DISCONNECTED;
	eSessionError									m_errorCode;
	std::string										m_errorStr;

	std::unique_ptr<StopWatch>						m_netClock;
	float											m_lastReceivedHostTime = 0.f;
	float											m_desiredClientTime = 0.f;
	float											m_currentClientTime = 0.f;
};