#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Logger.hpp"

TCPSocket::TCPSocket() {
	m_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

TCPSocket::~TCPSocket() {
}

bool TCPSocket::Listen(u16 port, u32 maxQueued) {
	sockaddr_in hostable;
	memset(&hostable, 0, sizeof(hostable));
	hostable.sin_family = AF_INET;
	hostable.sin_addr.S_un.S_addr = INADDR_ANY;
	hostable.sin_port = ::htons(port);

	int result = ::bind(m_handle, (sockaddr*)&hostable, sizeof(sockaddr));
	if (result == SOCKET_ERROR) {
		int error = ::WSAGetLastError();
		LogTaggedPrintf("error", "[%d] Could not bind.", error);
		return false;
	}

	result = ::listen(m_handle, maxQueued);
	if (result < 0) {
		int error = ::WSAGetLastError();
		LogErrorf("[%d] Could not listen.", error);
		return false;
	}

	std::vector<std::string> localAddrs;
	NetAddress_t::GetAddressForAllLocal(localAddrs);
	for (auto& it : localAddrs) {
		m_address = NetAddress_t(Stringf("%s:%u", it.c_str(), port));
		LogTaggedPrintf("net", "Hosting at: %s:%u", it.c_str(), port);
	}
	return true;
}

TCPSocket* TCPSocket::Accept() {
	// Set to non-blocking socket
	u_long non_blocking = 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);

	sockaddr_storage their_addr;
	int their_addrlen = sizeof(sockaddr_storage);
	TCPSocket* theirSocket = new TCPSocket();
	theirSocket->m_handle = ::accept(m_handle, (sockaddr*)&their_addr, &their_addrlen);
	if (theirSocket->m_handle == INVALID_SOCKET) {
		delete theirSocket;
		theirSocket = nullptr;
	}
	else{
		theirSocket->m_address.FromSockaddr((sockaddr*)&their_addr);
		LogTaggedPrintf("net", "Connection accepted: %s", theirSocket->m_address.ToString().c_str());

	}
	return theirSocket;
}


bool TCPSocket::Connect(const NetAddress_t& addr) {
	u_long non_blocking = 0;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);

	sockaddr_storage saddr;
	size_t addrlen;
	addr.ToSockaddr((sockaddr*)&saddr, &addrlen);
	int result = ::connect(m_handle, (sockaddr*)&saddr, (int)addrlen);
	if (result < 0) {
		if (HasFatalError()) {
		}
		return false;
	}
	m_address.FromSockaddr((sockaddr*)&saddr);
	LogTaggedPrintf("net", "Connected to: %s", m_address.ToString().c_str());

	// Set to non-blocking socket
	non_blocking = 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);
	return true;
}

size_t TCPSocket::Send(const std::string& msg) {
	return Send(const_cast<char*>(msg.c_str()), msg.length());
}

size_t TCPSocket::Send(const void* data, size_t length) {
	u_long non_blocking = 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);

	int sent = ::send(m_handle, (const char*)data, (int)length, 0);
	if (sent < 0) {
		if(HasFatalError()){
			Close();
		}
		return 0;
	}
	else if (sent == 0) {
		Close();
		return 0;
	}
	return sent;
}

size_t TCPSocket::Receive(void* buffer, const size_t maxBytes) {
	u_long non_blocking = 1;
	::ioctlsocket(m_handle, FIONBIO, &non_blocking);

	int receive = ::recv(m_handle, reinterpret_cast<char*>(buffer), (int)maxBytes, 0);
	if (receive < 0) {
		if(HasFatalError()){
			Close();
		}
		return 0;
	}
	else if(receive == 0){
		Close();
		return 0;
	}
	return receive;
}