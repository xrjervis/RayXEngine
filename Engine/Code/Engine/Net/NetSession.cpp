#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Net/NetSession.hpp"
#include "Engine/Net/NetConnection.hpp"

#pragma region Net Message Callbacks
//------------------------------Net Message Callbacks------------------------------------------
static bool OnPing(const NetMessage& msg, const NetSender_t& from) {
	std::string payload;
	msg.ReadString(payload);
	LogTaggedPrintf("netsession", "Received [ping] from %s : %s", from.address.ToString().c_str(), payload.c_str());

	NetMessageDefinition_t* msgDef = from.session->GetMessageDefinitionByName("pong");
	NetMessage pong(msgDef);

	if (from.netConn == nullptr) {
		LogTaggedPrintf("netsession", "No connections, responding to address directly");
		from.session->SendDirectMessage(from.address, pong);
	}
	else {
		LogTaggedPrintf("netsession", "Responding to connection.");
		from.netConn->Send(pong);
	}
	return true;
}

static bool OnPong(const NetMessage& msg, const NetSender_t& from) {
	UNUSED(msg);
	LogTaggedPrintf("netsession", "Received [pong] from %s", from.address.ToString().c_str());
	return true;
}

static bool OnAdd(const NetMessage& msg, const NetSender_t& from) {
	float a = 0.f;
	float b = 0.f;
	if (msg.ReadBytes(&a, 4) != 4) {
		return false;
	}
	if (msg.ReadBytes(&b, 4) != 4) {
		return false;
	}
	LogTaggedPrintf("netsession", "Received [add] from %s: %.5f + %.5f = %.5f", from.address.ToString().c_str(), a, b, a + b);

	NetMessageDefinition_t* msgDef = from.session->GetMessageDefinitionByName("add_response");
	NetMessage addresponse(msgDef);
	addresponse.WriteBytes(&a, 4);
	addresponse.WriteBytes(&b, 4);
	float result = a + b;
	addresponse.WriteBytes(&result, 4);

	if (from.netConn == nullptr) {
		LogTaggedPrintf("netsession", "No connections, responding to address directly");
		from.session->SendDirectMessage(from.address, addresponse);
	}
	else {
		LogTaggedPrintf("netsession", "Responding to connection.");
		from.netConn->Send(addresponse);
	}
	return true;
}

static bool OnAddResponse(const NetMessage& msg, const NetSender_t& from) {
	float a = 0.f;
	float b = 0.f;
	float result = 0.f;
	msg.ReadBytes(&a, 4);
	msg.ReadBytes(&b, 4);
	msg.ReadBytes(&result, 4);

	LogTaggedPrintf("netsession", "Received [add_response] from %s : %.5f + %.5f = %.5f", from.address.ToString().c_str(), a, b, result);
	return true;
}

static bool OnHeartbeat(const NetMessage& msg, const NetSender_t& from){
	UNUSED(msg);
	UNUSED(from);

//	LogTaggedPrintf("netsession", "Received [heartbeat] from %s", from.address.ToString().c_str());
	return true;
}

static bool OnUnreliableTest(const NetMessage& msg, const NetSender_t& from){
	UNUSED(from);
	int i = 0;
	int count = 0;
	msg.ReadBytes(&i, 4);
	msg.ReadBytes(&count, 4);
	LogTaggedPrintf("netsession", "(%d, %d)", i + 1, count);
	return true;
}

static bool OnTest(const NetMessage& msg, const NetSender_t& from) {
	if (from.netConn == nullptr) {
		LogTaggedPrintf("netsession", "Received a message that requires connection. Throw out!");
	}
	else {
		std::string payload;
		msg.ReadString(payload);
		LogTaggedPrintf("netsession", "Received [test] from %s : %s", from.address.ToString().c_str(), payload.c_str());
	}
	return true;
}

static bool OnReliableTest(const NetMessage& msg, const NetSender_t& from) {
	int i = 0;
	int count = 0;
	msg.ReadBytes(&i, 4);
	msg.ReadBytes(&count, 4);
	LogTaggedPrintf("netsession", "Reliable: (%d, %d)", i + 1, count);
	NetMessageDefinition_t* msgDef = from.session->GetMessageDefinitionByName("heartbeat");
	NetMessage heartbeat(msgDef);
	from.netConn->Send(heartbeat);
	return true;
}

static bool OnInOrderTest(const NetMessage& msg, const NetSender_t& from) {
	int i = 0;
	int count = 0;
	msg.ReadBytes(&i, 4);
	msg.ReadBytes(&count, 4);
	LogTaggedPrintf("netsession", "InOrder: (%d, %d)", i + 1, count);
	NetMessageDefinition_t* msgDef = from.session->GetMessageDefinitionByName("heartbeat");
	NetMessage heartbeat(msgDef);
	from.netConn->Send(heartbeat);
	return true;
}

static bool OnJoinRequest(const NetMessage& msg, const NetSender_t& from){
	UNUSED(msg);

	LogTaggedPrintf("netsession", "OnJoinRequest");
	NetSession* session = from.session;
	NetConnectionInfo_t connInfo;

	bool failFlag = false;
	if(!session->IsHost()) {
		failFlag = true;
		session->SetError(SESSION_ERROR_JOIN_DENIED_NOT_HOST, "");
	}
	else if (session->IsConnectionFull()){
		failFlag = true;
		session->SetError(SESSION_ERROR_JOIN_DENIED_FULL, "");
	}
	else {
		connInfo.address = from.address;
		if(session->DoesNetAddressExist(from.address)){
			// ignore
			LogWarningf("Join request ignored.");
			return true;
		}
		else{
			// On the host side, connection's slot index is equal session index
			connInfo.index = (u8)session->GetAvailableConnectionSlot();
			NetConnection* conn = session->CreateConnection(connInfo);
			if(conn == nullptr){
				failFlag = true;
				session->SetError(SESSION_ERROR_JOIN_DENIED_CLOSED, "");
			}
			else{
				conn->m_state = CONNECTION_CONNECTED;
				session->BindConnection(connInfo.index, conn);
				// Send Accept
				NetMessageDefinition_t* msgDef = session->GetMessageDefinitionByIndex(NETCOREMSG_JOIN_ACCEPT);
				NetMessage joinAcceptMsg(msgDef);
				joinAcceptMsg.WriteBytes(&connInfo.index, 1);
				conn->Send(joinAcceptMsg);

				// Send Finished
				msgDef = session->GetMessageDefinitionByIndex(NETCOREMSG_JOIN_FINISHED);
				NetMessage joinFinishedMsg(msgDef);
				conn->Send(joinFinishedMsg);

				// Send Sync Time
				msgDef = session->GetMessageDefinitionByIndex(NETCOREMSG_SYNCTIME);
				NetMessage syncTimeMsg(msgDef);
				conn->Send(syncTimeMsg);
			}
		}
	}

	if(failFlag){
		// Send Deny
		NetMessageDefinition_t* msgDef = session->GetMessageDefinitionByIndex(NETCOREMSG_JOIN_DENY);
		NetMessage joinDenyMsg(msgDef);
		session->SendDirectMessage(from.address, joinDenyMsg);
	}
	return true;
}

static bool OnJoinDeny(const NetMessage& msg, const NetSender_t& from) {
	UNUSED(msg);

	LogTaggedPrintf("netsession", "OnJoinDeny");
	NetSession* session = from.session;
	session->CloseAllConnections();
	return true;
}

static bool OnJoinAccept(const NetMessage& msg, const NetSender_t& from) {
	LogTaggedPrintf("netsession", "OnJoinAccept");

	NetSession* session = from.session;
	if(session->m_state == SESSION_CONNECTING){
		session->m_state = SESSION_JOINING;
		u8 sessionIndex;
		msg.ReadBytes(&sessionIndex, 1);
		session->m_myConnection->m_info.index = sessionIndex;
		session->m_connectionIndex = sessionIndex;
	}
	return true;
}

static bool OnNewConnection(const NetMessage& msg, const NetSender_t& from) {
	LogTaggedPrintf("netsession", "OnNewConnection");
	from.netConn->m_state = CONNECTION_READY;
	NetMessageDefinition_t* msgDef = from.session->GetMessageDefinitionByName("heartbeat");
	NetMessage heartbeat(msgDef);
	from.netConn->Send(heartbeat);

	std::string name;
	msg.ReadString(name);
	// On connection join callback (e.g. Create player for connection in the game)
	if(from.session->m_onJoinCallback){
		from.session->m_onJoinCallback(from.netConn, name);
	}
	return true;
}

static bool OnJoinFinished(const NetMessage& msg, const NetSender_t& from) {
	UNUSED(msg);

	LogTaggedPrintf("netsession", "OnJoinFinished");
	NetSession* session = from.session;
	if(session->m_state == SESSION_JOINING){
		session->m_state = SESSION_READY;
		session->m_hostConnection->m_state = CONNECTION_READY;
		session->m_myConnection->m_state = CONNECTION_READY;

		NetMessageDefinition_t* msgDef = session->GetMessageDefinitionByIndex(NETCOREMSG_NEW_CONNECTION);
		NetMessage newConnStateMsg(msgDef);
		newConnStateMsg.WriteString(session->m_name);
		session->m_hostConnection->Send(newConnStateMsg);
	}
	return true;
}

static bool OnUpdateConnectionState(const NetMessage& msg, const NetSender_t& from){
	UNUSED(msg);
	UNUSED(from);

	return true;
}

static bool OnHangUp(const NetMessage& msg, const NetSender_t& from) {
	UNUSED(msg);

	LogTaggedPrintf("netsession", "OnHangUp");
	NetSession* session = from.session;
	if(session->m_state == SESSION_READY){
		from.netConn->m_isMarkedToDestory = true;
	}
	return true;
}

static bool OnSyncTime(const NetMessage& msg, const NetSender_t& from) {
	// Only clients are supposed to receive this message
	NetSession* session = from.session;
	if(session->IsHost()){
		return false;
	}
	float hostTime = 0.f;
	if (msg.ReadBytes(&hostTime, 4)) {
		// If it is more recent then the last time update, process it (otherwise early return)
		if(hostTime <= session->m_lastReceivedHostTime){
			return true;
		}

		session->m_lastReceivedHostTime = hostTime + from.netConn->m_rtt * 0.5f;
		session->m_desiredClientTime = session->m_lastReceivedHostTime;
		// if it's the first time received host time, set current client time
		if(session->m_currentClientTime == 0.f){
			session->m_currentClientTime = session->m_desiredClientTime;
			session->m_myConnection->m_lastReceivedTime = session->m_currentClientTime;
			session->m_hostConnection->m_lastReceivedTime = session->m_currentClientTime;
		}

	}
	return true;
}

//---------------------------------------------------------------------------------------------
#pragma endregion

NetSession::NetSession() {
	m_socket = new UDPSocket();
	LogPrintf("NetSession started at %s.", m_socket->m_address.ToString().c_str());
	RegisterCoreMessages();
	RegisterGameMessages();

	m_netObjectSystem.m_owningSession = this;
}

NetSession::~NetSession() {
	if(m_myConnection){
		Disconnect(m_myConnection);
	}
	for(auto it : m_netMessageDefinitions){
		SAFE_DELETE(it.second);
	}
	SAFE_DELETE(m_socket);
}

void NetSession::RegisterCoreMessages() {
	RegisterMessageDefinition(NETCOREMSG_PING, "ping", OnPing, NETMSGOPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NETCOREMSG_PONG, "pong", OnPong, NETMSGOPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NETCOREMSG_HEARTBEAT, "heartbeat", OnHeartbeat, NETMSGOPTION_HEARTBEAT);
	RegisterMessageDefinition(NETCOREMSG_JOIN_REQUEST, "join_request", OnJoinRequest, NETMSGOPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NETCOREMSG_JOIN_DENY, "join_deny", OnJoinDeny, NETMSGOPTION_CONNECTIONLESS);
	RegisterMessageDefinition(NETCOREMSG_JOIN_ACCEPT, "join_accept", OnJoinAccept, NETMSGOPTION_RELIABLE_INORDER);
	RegisterMessageDefinition(NETCOREMSG_NEW_CONNECTION, "new_connection", OnNewConnection, NETMSGOPTION_RELIABLE_INORDER);
	RegisterMessageDefinition(NETCOREMSG_JOIN_FINISHED, "join_finished", OnJoinFinished, NETMSGOPTION_RELIABLE_INORDER);
	RegisterMessageDefinition(NETCOREMSG_UPDATE_CONN_STATE, "update_conn_state", OnUpdateConnectionState, NETMSGOPTION_RELIABLE_INORDER);
	RegisterMessageDefinition(NETCOREMSG_HANGUP, "hang_up", OnHangUp, NETMSGOPTION_NONE);
	RegisterMessageDefinition(NETCOREMSG_SYNCTIME, "sync_time", OnSyncTime, NETMSGOPTION_NONE);
}

void NetSession::RegisterGameMessages() {
	RegisterMessageDefinition(NETGAMEMSG_TEST, "test", OnTest);
	RegisterMessageDefinition(NETGAMEMSG_UNRELIABLE_TEST, "unreliable_test", OnUnreliableTest);
	RegisterMessageDefinition(NETGAMEMSG_RELIABLE_TEST, "reliable_test", OnReliableTest, NETMSGOPTION_RELIABLE);
	RegisterMessageDefinition(NETGAMEMSG_INORDER_TEST, "inorder_test", OnInOrderTest, NETMSGOPTION_RELIABLE_INORDER);

	RegisterMessageDefinition("add", OnAdd);
	RegisterMessageDefinition("add_response", OnAddResponse);
}

void NetSession::FinalizeNetMessageIndex() {
	u8 startIdx = 0;
	for(size_t idx = 0; idx < 255; ++idx){
		if(m_fixedNetmessageIndexArray[idx] == false){
			startIdx = idx;
			break;
		}
	}

	for(auto& it : m_netMessageDefinitions){
		if(m_fixedNetmessageIndexArray[it.second->index] == false){
			// if it's not fixed index
			while (m_fixedNetmessageIndexArray[startIdx++] == true) {
			}
			it.second->index = startIdx - 1;
		}
	}
}

void NetSession::RegisterMessageDefinition(const std::string& name, NetMessage_cb cb, eNetMessageOption options) {
	if (GetMessageDefinitionByName(name) != nullptr) {
		LogErrorf("Message definition name has been registered!");
		return;
	}

	NetMessageDefinition_t* msgDef = new NetMessageDefinition_t();
	msgDef->index = 0xff;
	msgDef->callback = cb;
	msgDef->options = options;
	m_netMessageDefinitions[name] = msgDef;
	FinalizeNetMessageIndex();

	LogTaggedPrintf("netsession", "Register message [%s] to index [%u]", name.c_str(), msgDef->index);
}

void NetSession::RegisterMessageDefinition(u8 idx, const std::string& name, NetMessage_cb cb, eNetMessageOption options /*= NETMSGOPTION_NONE*/) {
	if (GetMessageDefinitionByName(name) != nullptr) {
		LogErrorf("Message definition name has been registered!");
		return;
	}

	NetMessageDefinition_t* msgDef = new NetMessageDefinition_t();
	msgDef->index = idx;
	msgDef->callback = cb;
	msgDef->options = options;
	m_netMessageDefinitions[name] = msgDef;
	m_fixedNetmessageIndexArray[idx] = true;

	LogTaggedPrintf("netsession", "Register message [%s] to index [%u]", name.c_str(), msgDef->index);
}

NetMessageDefinition_t* NetSession::GetMessageDefinitionByName(const std::string& name) {
	return m_netMessageDefinitions[name];
}

NetMessageDefinition_t* NetSession::GetMessageDefinitionByIndex(u8 idx) {
	for(auto it : m_netMessageDefinitions){
		if(it.second->index == idx){
			return it.second;
		}
	}
	return nullptr;
}


NetConnection* NetSession::GetConnection(u8 idx) {
	if(idx >= MAX_CONNECTIONS){
		return nullptr;
	}
	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		NetConnection* conn = m_connections[i];
		if(conn && conn->m_info.index == idx){
			return conn;
		}
	}
	return nullptr;
}

NetConnection* NetSession::GetConnection(const NetAddress_t& addr) {
	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		NetConnection* conn = m_connections[i];
		if (conn && conn->m_info.address.ToString() == addr.ToString()) {
			return conn;
		}
	}
	return nullptr;
}

void NetSession::CloseAllConnections() {
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i){
		DestroyConnection(m_connections[i]);
		m_connections[i] = nullptr;
	}
	m_hostConnection = nullptr;
	m_myConnection = nullptr;
	m_connectionIndex = INVALID_CONN_INDEX;
	m_joinRequestTimer = 0.f;
	m_sessionConnectingTimeoutTimer = 0.f;
	m_state = SESSION_DISCONNECTED;
	m_socket->Close();
}

void NetSession::ProcessIncomingPackets() {
	if(m_socket->IsClosed()){
		return;
	}
	while (true) {
		// Receiving a NetPacket
		NetAddress_t incomingAddr;
		u8 buffer[PACKET_MTU];
		size_t recvCount = m_socket->Receive(incomingAddr, buffer, PACKET_MTU);
		if (recvCount == 0) {
			return;
		}
		else {
			NetPacket* packet = new NetPacket(buffer);
			PacketHeader_t* header = new PacketHeader_t();
			if (packet->ReadHeader(*header)) {
				NetConnection* conn = nullptr;
				conn = GetConnection(header->senderConnectionIdx);
				packet->m_sender.address = incomingAddr;
				packet->m_sender.session = this;
				packet->m_sender.netConn = conn;
				if (conn) {
					conn->Receive(packet, header);
					conn->Process();
				}
				else {
					ProcessDirectMessage(packet, header);
				}
			}
			else {
				LogWarningf("Read packet header failed");
			}
		}
	}
}

void NetSession::ProcessOutgoingPackets() {
	if (m_socket->IsClosed()) {
		return;
	}
	// For loop all connections
	// Create a NetPacket for each connection that has queued messages
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		NetConnection* conn = m_connections[i];
		if(conn == nullptr){
			continue;
		}
		if(conn->m_tickStopWatch->CheckAndReset()){
			conn->Flush();
		}
	}
}

void NetSession::SendDirectMessage(const NetAddress_t& addr, const NetMessage& msg) {
	PacketHeader_t header;
	header.senderConnectionIdx = m_myConnection->m_info.index;
	header.messageCount = 1;
	NetPacket packet;
	packet.WriteHeader(header);
	packet.WriteMessage(msg);
	m_socket->Send(addr, packet.GetBuffer(), packet.GetWrittenByteCount());
}

void NetSession::SendDirectMessage(u8 connectionIdx, const NetMessage& msg) {
	SendDirectMessage(m_connections[connectionIdx]->m_info.address, msg);
}

void NetSession::ProcessDirectMessage(NetPacket* packet, PacketHeader_t* header) {
	LogTaggedPrintf("netsession", "ProcessDirectMessage");
	// Read messages
	for (u8 msgIdx = 0; msgIdx < header->messageCount; ++msgIdx) {
		NetMessage msg;
		if (!packet->ReadMessage(msg)) {
			LogErrorf("Receive %u garbage messages:", packet->GetWrittenByteCount());
// 			for (size_t i = 0; i < packet.GetWrittenByteCount(); ++i) {
// 				LogErrorf("[0x%x]", ((u8*)packet.GetBuffer())[i]);
// 			}
			return;
		}

		NetMessageDefinition_t* msgDef = GetMessageDefinitionByIndex(msg.m_index);
		// Execute NetMessage
		if (msgDef) {
			if(msgDef->options == NETMSGOPTION_CONNECTIONLESS){
				msgDef->callback(msg, packet->m_sender);
			}
			else{
				LogWarningf("Message [%u] is not connectionless. Throw out!", msgDef->index);
			}
		}
		else {
			LogWarningf("Cannot find message definition for index [%d]", msg.m_index);
		}
	}

	delete packet;
	delete header;
}


void NetSession::SetSimLoss(float lossChance) {
	lossChance = Clamp01(lossChance);
	m_lossChance = lossChance;
}

void NetSession::SetSimLatency(float min, float max) {
	m_minLatency = min;
	m_maxLatency = max;
}

void NetSession::SetHeartbeatRate(float rate) {
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i){
		NetConnection* conn = m_connections[i];
		if (conn == nullptr) {
			continue;
		}
		conn->m_heartbeatStopWatch->SetTimer(rate);
		conn->m_heartbeatStopWatch->Reset();
	}
}

void NetSession::Host( u16 port, u16 portRange /*= DEFAULT_PORT_RANGE*/) {
	if(m_state != SESSION_DISCONNECTED){
		LogErrorf("Host failed: Session is not in disconnected state.");
	}
	else{
		std::string bindable = NetAddress_t::GetBindableAddress(port);
		NetAddress_t netAddr(bindable);

		if (!m_socket->Bind(netAddr, portRange)) {
			LogErrorf("Host failed: Session cannot bind socket.");
			SetError(SESSION_ERROR_BIND_SOCKET_FAILED, "");
		}
		else {
			m_state = SESSION_BOUND;
			NetConnectionInfo_t connInfo;
			connInfo.playerID = CreateOrGetStringId(m_name);
			connInfo.address = netAddr;
			connInfo.index = 0; // Host is always at index[0]

			NetConnection* conn = CreateConnection(connInfo);
			BindConnection(0, conn);
			m_myConnection = conn;
			m_hostConnection = conn;
			m_connectionIndex = 0u;
			conn->m_state = CONNECTION_READY;
			m_state = SESSION_READY;
			m_netClock = std::make_unique<StopWatch>(g_theMasterClock.get());

			LogTaggedPrintf("netsession", "Socket bound: %s", netAddr.ToString().c_str());
		}
	}

}

void NetSession::Join(const NetConnectionInfo_t& hostInfo) {
	if (m_state != SESSION_DISCONNECTED) {
		LogErrorf("Join failed: Session is not in disconnected state.");
		return;
	}
	// Bind the socket using host's port
	u16 startingPort = hostInfo.address.m_port;
	std::string bindable = NetAddress_t::GetBindableAddress(startingPort);
	NetAddress_t netAddr(bindable);
	m_socket->Bind(netAddr, DEFAULT_PORT_RANGE);

	// Create connection for the host
	NetConnection* hostConn = CreateConnection(hostInfo);
	BindConnection(0u, hostConn);
	hostConn->m_state = CONNECTION_CONNECTED;
	m_hostConnection = hostConn;

	// Create connection for myself
	NetConnectionInfo_t connInfo;
	connInfo.address = m_socket->m_address;
	connInfo.index = INVALID_CONN_INDEX;
	NetConnection* conn = CreateConnection(connInfo);
	u8 slotIndex = GetAvailableConnectionSlot();
	BindConnection(slotIndex, conn);
	m_myConnection = conn;
	m_myConnection->m_state = CONNECTION_CONNECTED;

	m_joinRequestTimer = 0.f;
	m_sessionConnectingTimeoutTimer = 0.f;
	m_state = SESSION_CONNECTING;
}

void NetSession::Update() {
	if (m_socket->IsClosed()) {
		return;
	}

	if(m_state == SESSION_CONNECTING){
		m_joinRequestTimer += g_theMasterClock->GetDeltaSeconds();
		m_sessionConnectingTimeoutTimer += g_theMasterClock->GetDeltaSeconds();
		// Timeout
		if (m_sessionConnectingTimeoutTimer > DEFAULT_CONNECTION_TIMEOUT) {
			CloseAllConnections();
			SetError(SESSION_ERROR_JOIN_TIMEOUT, "");
			LogErrorf("Join Time Out!");
		}
		// Send join request every 0.1 seconds
		if (m_joinRequestTimer > 0.1f) {
			m_joinRequestTimer = 0.f;
			NetMessageDefinition_t* msgDef = GetMessageDefinitionByIndex(NETCOREMSG_JOIN_REQUEST);
			NetMessage joinRequestMsg(msgDef);
			m_hostConnection->Send(joinRequestMsg);
		}
	}
	else if(m_state == SESSION_JOINING){
		// Do nothing
	}
	else if(m_state == SESSION_READY) {
		// Loop over all connections
		for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
			NetConnection* conn = m_connections[i];
			if (conn == nullptr) {
				continue;
			}
			if(conn->m_isMarkedToDestory){
				if(IsHost()){
					if(m_onLeaveCallback){
						m_onLeaveCallback(conn);
					}
				}
				Disconnect(conn);
				continue;
			}
			for (auto& msg : conn->m_unconfirmedReliables) {
				msg.m_resendTime += g_theMasterClock->GetDeltaSeconds();
			}
			if(conn->IsReady() && GetNetTimeInSeconds() - conn->m_lastReceivedTime > DEFAULT_CONNECTION_TIMEOUT){
				conn->m_isMarkedToDestory = true;
			}
			
			//bool canSendHeartbeat = conn->m_heartbeatStopWatch->CheckAndReset();
			bool canSendHeartbeat = true;

			if (canSendHeartbeat) {
				NetMessageDefinition_t* msgDef = GetMessageDefinitionByName("heartbeat");
				NetMessage heartbeat(msgDef);
				conn->Send(heartbeat);

				// Only host can sync time
				if(IsHost()){
					msgDef = GetMessageDefinitionByIndex(NETCOREMSG_SYNCTIME);
					NetMessage syncTimeMsg(msgDef);
					float currentNetTime = GetNetTimeInSeconds();
					syncTimeMsg.WriteBytes(&currentNetTime, 4);
					conn->Send(syncTimeMsg);
				}
			}
		}

		// Update client's time based on host's net clock
		if(!IsHost()){
			m_desiredClientTime += g_theMasterClock->GetDeltaSeconds();
			if(m_desiredClientTime > m_currentClientTime + g_theMasterClock->GetDeltaSeconds()){
				m_currentClientTime += (1.f + MAX_NET_TIME_DILATION) * g_theMasterClock->GetDeltaSeconds();
			}
			else{
				m_currentClientTime += (1.f - MAX_NET_TIME_DILATION) * g_theMasterClock->GetDeltaSeconds();

			}
		}
	}
}

void NetSession::SetError(eSessionError error, const std::string& errorStr) {
	m_errorCode = error;
	m_errorStr = errorStr;
}

void NetSession::ClearError() {
	m_errorStr = "";
	m_errorCode = SESSION_OK;
}

eSessionError NetSession::GetLastError(std::string& outStr) {
	outStr = m_errorStr;
	eSessionError errorCode = m_errorCode;
	ClearError();
	return errorCode;
}

void NetSession::Render() const {
// 	g_theRenderer->SetFont(L"Consolas", 15.f);
// 
// 	g_theRenderer->DrawTextInBox(Stringf("Net clock time: %.2f(s)", GetNetTimeInSeconds()), Window::Get(0)->GetOrtho(), Rgba::YELLOW, DWRITE_TEXT_ALIGNMENT_TRAILING);
// 	g_theRenderer->DrawTextInBox(
// 		Stringf("%2s %-5s %-20s %-8s %-5s %-8s %-8s %-8s %-8s %18s",
// 			"--", "idx", "address", "rtt(s)", "loss", "lrcv(s)", "lsnt(s)", "snt_ack", "rcv_ack", "bitfield"),
// 		AABB2(Window::Get(0)->GetOrtho().mins, Window::Get(0)->GetOrtho().maxs - Vector2(0.f, 15.f)),
// 		Rgba::WHITE,
// 		DWRITE_TEXT_ALIGNMENT_TRAILING);
// 	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
// 		std::string mySymbol;
// 		NetConnection* conn = m_connections[i];
// 		if(conn == nullptr){
// 			continue;
// 		}
// 
// 		std::bitset<16> bs(conn->m_previousReceivedAckBitfield);
// 		g_theRenderer->DrawTextInBox(
// 			Stringf("%2s %-5u %-25s %-8.2f %-8.2f %-8.3f %-8.3f %-8u %-8u %-18s ",
// 				mySymbol.c_str(), conn->m_info.index, conn->m_info.address.ToString().c_str(), conn->m_rtt, conn->m_loss, 
// 				GetNetTimeInSeconds() - conn->m_lastReceivedTime, 
// 				GetNetTimeInSeconds() - conn->m_lastSendTime, conn->m_nextSentAck, conn->m_lastReceivedAck, bs.to_string().c_str()),
// 			AABB2(Window::Get(0)->GetOrtho().mins, Vector2(Window::Get(0)->GetOrtho().maxs.x, Window::Get(0)->GetOrtho().maxs.y - 15.f * (float)(i + 2))),
// 			Rgba::WHITE,
// 			DWRITE_TEXT_ALIGNMENT_TRAILING);
// 	}
}

bool NetSession::IsConnectionFull() const {
	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		if(m_connections[i] == nullptr){
			return false;
		}
	}
	return true;
}

size_t NetSession::GetAvailableConnectionSlot() const {
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i){
		if(m_connections[i] == nullptr){
			return i;
		}
	}
	return 0;
}

bool NetSession::DoesNetAddressExist(const NetAddress_t& addr) const {
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i){
		if(m_connections[i] && m_connections[i]->m_info.address.ToString() == addr.ToString()){
			return true;
		}
	}
	return false;
}

NetConnection* NetSession::CreateConnection(const NetConnectionInfo_t& info) {
	if(m_socket->IsClosed()){
		return nullptr;
	}

	NetConnection* conn = new NetConnection();
	conn->m_lastSendTime = GetNetTimeInSeconds();
	conn->m_lastReceivedTime = GetNetTimeInSeconds();
	conn->m_tickStopWatch = std::make_unique<StopWatch>(g_theMasterClock.get());
	conn->m_tickStopWatch->SetTimer(m_sendRate);
	conn->m_heartbeatStopWatch = std::make_unique<StopWatch>(g_theMasterClock.get());
	conn->m_heartbeatStopWatch->SetTimer(5.f);
	conn->m_info = info;
	conn->m_owningSession = this;
	return conn;
}

void NetSession::DestroyConnection(NetConnection* conn) {
	SAFE_DELETE(conn);
}

void NetSession::BindConnection(u8 idx, NetConnection* conn) {
	if(idx >= MAX_CONNECTIONS || m_connections[idx]){
		LogErrorf("Bind connection failed.");

	}
	else {
		m_connections[idx] = conn;
	}
}

void NetSession::Disconnect(NetConnection* conn) {
	if(conn == m_myConnection){
		for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
			if (m_connections[i] && m_connections[i] != m_myConnection) {
				// Inform other connections
				NetMessageDefinition_t* msgDef = GetMessageDefinitionByIndex(NETCOREMSG_HANGUP);
				NetMessage hangupMsg(msgDef);
				m_connections[i]->Send(hangupMsg);
				// Flush all messages
				m_connections[i]->Flush();
			}
		}
		// Close all connections
		CloseAllConnections();
	}
	else if (conn == m_hostConnection){
		// Inform other connections
		NetMessageDefinition_t* msgDef = GetMessageDefinitionByIndex(NETCOREMSG_HANGUP);
		NetMessage hangupMsg(msgDef);
		m_hostConnection->Send(hangupMsg);
		// Flush all messages
		m_hostConnection->Flush();
		CloseAllConnections();
	}
	else {
		for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
			if(conn == m_connections[i]){
				DestroyConnection(m_connections[i]);
				m_connections[i] = nullptr;
			}
		}
	}
}

float NetSession::GetNetTimeInSeconds() const {
	if(IsHost()){
		return m_netClock->GetElapsedTime();
	}
	else {
		return m_currentClientTime;
	}
}

void NetSession::SendToAllOthers(const NetMessage& msg, eConnectionState connState) {
	for(size_t i = 0; i < MAX_CONNECTIONS; ++i){
		NetConnection* conn = m_connections[i];
		if(conn){
			if(conn != m_myConnection && conn->m_state == connState){
				conn->Send(msg);
			}
		}
	}
}
