#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Net/NetMessage.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/ThreadSafeContainer.hpp"
#include <tuple>

class NetSession;
class NetConnection;

// IPv4 Header Size: 20B
// IPv6 Header Size: 40B
// TCP Header Size: 20B-60B
// UDP Header Size: 8B 
// Ethernet: 28B, but MTU is already adjusted for it
// so packet size is 1500 - 40 - 8 => 1452B (why?)
constexpr size_t ETHERNET_MTU = 1500;  // maximum transmission unit - determined by hardware part of OSI model.
// 1500 is the MTU of EthernetV2, and is the minimum one - so we use it; 
constexpr size_t PACKET_MTU = (ETHERNET_MTU - 40 - 8);

constexpr u16 INVALID_PACKET_ACK = 0xFFFF;

constexpr size_t PACKET_HEADER_SIZE = 8;
volatile struct PacketHeader_t {
	u8	senderConnectionIdx; // conn idx of the sender of this packet
	u16 ack;
	u16 lastReceivedAck;
	u16 previousReceivedAckBitfield;
	u8	messageCount; // number of messages in this container
};

struct NetSender_t {
	NetSession*		session = nullptr;
	NetConnection*	netConn = nullptr;
	NetAddress_t	address;
};

// A packet is a package sent over the network
// It allows you to write/read data to it
// also contains who the recipient and/or sender are
// Contains extra functionality for writing headers/messages. 
// Use little endian
class NetPacket : public BytePacker{
public:
	NetPacket();
	NetPacket(void* buffer);
	NetPacket(const NetPacket&) = default;
	NetPacket& operator=(const NetPacket&) = default;
	NetPacket(NetPacket&& other);
	NetPacket& operator=(NetPacket&& other);

	virtual ~NetPacket();

	bool WriteHeader(const PacketHeader_t& header);
	bool UpdateHeader(const PacketHeader_t& header);
	bool ReadHeader(PacketHeader_t& out) const;

	bool WriteMessage(const NetMessage& msg);
	bool ReadMessage(NetMessage& out) const;

public:
	mutable NetSender_t		m_sender;
};