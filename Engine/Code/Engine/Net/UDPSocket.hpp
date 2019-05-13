#pragma once
#include "Engine/Net/Socket.hpp"

constexpr u16 UDP_PORT = 10084u;
constexpr u16 DEFAULT_PORT_RANGE = 16u;

class UDPSocket : public Socket {
public:
	UDPSocket();
	virtual ~UDPSocket();

	bool Bind(const NetAddress_t& addr,		// address I want to listen for traffic on
		u16 portRange = 0u);				// how many additional ports to bind on (so you bind from [addr.port,addr.port + port_range])

	size_t Send(const NetAddress_t& addr, const void* data, const size_t byteCount);
	size_t Receive(NetAddress_t& outNetAddr, void *buffer, const size_t maxByteCount);
};
