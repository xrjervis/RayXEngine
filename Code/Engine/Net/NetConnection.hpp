#pragma once
#include <queue>
#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/NetConnectionInfo.hpp"
#include "Engine/Core/StopWatch.hpp"
#include <memory>
#include <mutex>
#include <tuple>

class NetSession;

constexpr u8 INVALID_CONNECTION = 0xFF;
constexpr u16 MAX_RELIABLES_PER_PACKET = 32;

struct PacketTracker_t{
	void TrackReliable(u16 id){
		if(reliableCount < MAX_RELIABLES_PER_PACKET ){
			reliableIDs[reliableCount++] = id;
		}
	}

	float sendTime = 0.f;
	u16	reliableIDs[MAX_RELIABLES_PER_PACKET];
	u16 reliableCount = 0U;
	u16 ack = INVALID_PACKET_ACK;
};

class NetConnection {
public:
	NetConnection();
	~NetConnection();

// 	bool	IsMe() const;
// 	bool	IsHost() const;
// 	bool	IsClient() const;

	inline bool IsConnected() const { return m_state >= CONNECTION_CONNECTED; }
	inline bool IsDisconnected() const { return m_state == CONNECTION_DISCONNECTED; }
	inline bool IsReady() const { return m_state == CONNECTION_READY; }

	void	Send(const NetMessage& msg);
	void	Flush();	// flush queued messages

	void	Receive(NetPacket* packet, PacketHeader_t* header);
	void	Process(); // process rcvd packets;


private:
	u16		GetNextAckToSend();
	u16		GetNextReliableID();
	void	AddPacketTracker(PacketTracker_t* tracker);
	void	ConfirmPacket(PacketHeader_t* header);
	void	ProcessMessagesOnPacket(PacketHeader_t* header, NetPacket* packet);
	void	UpdateReceivedAcks(PacketHeader_t* header);

	bool	HasReceivedReliableID(u16 id);
	void	AddReceivedReliableID(u16 id);
	void	ProcessOutOfOrderMessages(NetSender_t* sender);
	void	AddOutOfOrderMessage(NetMessage* msg);

public:
	bool						m_isMarkedToDestory = false;

	// Send - Wait for Flush
	std::vector<PacketTracker_t*> m_packetTrackerList;

	// Unreliables
	std::queue<NetMessage>		m_unsentUnreliables;

	// Reliable traffic
	std::vector<NetMessage>		m_unconfirmedReliables;
	std::vector<NetMessage>		m_unsentReliables;
	std::vector<u16>			m_receivedReliableIDs;
	u16							m_nextSentReliableId = 0u;

	// In order traffic
	std::vector<NetMessage*>	m_outOfOrderMessages;
	u16							m_nextExpectedReliableId = 0u;

	// Receive - Wait for Process
// 	std::vector<NetPacket*>		m_receivedPackets;
// 	std::vector<float>			m_receivedTimestamps;
	std::vector<std::tuple<float, NetPacket*, PacketHeader_t*>> m_receivedPackets;

	NetConnectionInfo_t			m_info;
	eConnectionState			m_state;
	NetSession*					m_owningSession = nullptr;
	float						m_sendRate = 0.f; //Hz default: send as fast as possible
	std::unique_ptr<StopWatch>	m_tickStopWatch;
	std::unique_ptr<StopWatch>	m_heartbeatStopWatch;


	// receiving - updated during a process_packet
	u16							m_nextSentAck = INVALID_PACKET_ACK; // sending - updated during a send/flush
	u16							m_lastReceivedAck = INVALID_PACKET_ACK;
	u16							m_previousReceivedAckBitfield = 0u; // stores previous 16 recvd acks;

	// Analytics
	size_t						m_totalSentPackets = 0;
	size_t						m_totalLostPackets = 0;
	float						m_lastSendTime = 0.f;
	float						m_lastReceivedTime = 0.f;

	// note these variables are unrelated to the debug simulation on the session
	// but will end up reflecting those numbers.
	float						m_loss = 0.0f;       // loss rate we perceive to this connection
	float						m_rtt = 0.0f;        // latency perceived on this connection
};