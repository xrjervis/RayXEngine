#pragma once
#include "Engine/Net/Socket.hpp"

class TCPSocket : public Socket {
public:
	TCPSocket();
	virtual ~TCPSocket();

	bool Listen(u16 port, u32 maxQueued);
	TCPSocket* Accept();
	bool Connect(const NetAddress_t& addr);
	size_t Send(const std::string& msg);
	size_t Send(const void* data, size_t length);
	size_t Receive(void* buffer, const size_t maxBytes);
};