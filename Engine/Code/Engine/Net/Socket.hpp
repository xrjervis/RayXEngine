#pragma once
#include "Engine/Net/NetAddress.hpp"

enum eSocketOptionBit : uint {
	SOCKET_OPTION_BLOCKING = BIT_FLAG(0),

	// some other options that may be useful to support
	// SOCKET_OPTION_BROADCAST - socket can broadcast messages (if supported by network)
	// SOCKET_OPTION_LINGER - wait on close to make sure all data is sent
	// SOCKET_OPTION_NO_DELAY - disable nagle's algorithm
};
typedef uint eSocketOptions;

// Rip out things from TCPSocket and put it to a base leve; 
class Socket {
public:
	Socket();
	virtual ~Socket();

	void				SetBlocking(bool blocking);
	void				Close();
	bool				IsClosed() const;

public:
	NetAddress_t		m_address;					// address assocated with this socket; 
	SOCKET				m_handle = INVALID_SOCKET;	// initialized to INVALID_SOCKET

	// used if you want to set options while closed, 
	// used to keep track so you can apply_options() after the socket
	// gets created; 
	eSocketOptions		m_options;
};

// fatal errors cause the socket to close; 
extern bool HasFatalError();