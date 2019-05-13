#include "Engine/Net/Socket.hpp"


Socket::Socket() {
}

Socket::~Socket() {
	Close();
}

void Socket::SetBlocking(bool blocking) {
	u_long non_blocking = !blocking;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);
}

void Socket::Close() {
	::closesocket(m_handle);
	m_handle = INVALID_SOCKET;
}

bool Socket::IsClosed() const {
	return m_handle == INVALID_SOCKET;
}

extern bool HasFatalError() {
	int error = ::WSAGetLastError();
	if (error == WSAEWOULDBLOCK || error == WSAEMSGSIZE || error == WSAECONNRESET) {
		return false;
	}
	else {
		return true;
	}
}
