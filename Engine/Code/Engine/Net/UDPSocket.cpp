#include "Engine/Net/UDPSocket.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

UDPSocket::UDPSocket() {
}

UDPSocket::~UDPSocket() {

}

bool UDPSocket::Bind( const NetAddress_t& addr, u16 portRange /*= 0U*/) {
	NetAddress_t currentAddr = addr;
	// create the socket 
	SOCKET my_socket = ::socket(
		AF_INET,		// IPv4 to send...
		SOCK_DGRAM,		// ...Datagrams... 
		IPPROTO_UDP);	// ...using UDP.

	GUARANTEE_OR_DIE(my_socket != INVALID_SOCKET, "");

	//try to bind all ports within the range.   
	for(size_t i = 0; i < portRange; ++i) {
		sockaddr_storage sock_addr;
		size_t sock_addr_len;
		currentAddr.ToSockaddr((sockaddr*)&sock_addr, &sock_addr_len);

		// try to bind - if it succeeds - great.  If not, try the next port in the range.
		int result = ::bind(my_socket, (sockaddr*)&sock_addr, (int)sock_addr_len);
		if (0 == result) {
			m_handle = my_socket;
			m_address = currentAddr;
			SetBlocking(false);
			return true;
		}
		else{
			currentAddr.m_port++;
		}
	}
	
	return false;
}

size_t UDPSocket::Send(const NetAddress_t& addr, const void* data, const size_t byteCount) {

	if (IsClosed()) {
		return false;
	}

	sockaddr_storage sock_addr;
	size_t addr_len;
	addr.ToSockaddr((sockaddr*)&sock_addr, &addr_len);

	int sent = ::sendto(m_handle, //socket we are sending from
		(char const*)data, // data we want to send
		(int)byteCount, // bytes to send
		0, // unused flags 
		(sockaddr*)&sock_addr, //address we're sending to
		(int)addr_len);

	if (sent > 0) {
		GUARANTEE_RECOVERABLE(sent == byteCount, "");
		return (size_t)sent;
	}
	else {
		if(HasFatalError()){
			Close();
		}
		return 0U;
	}
}

size_t UDPSocket::Receive(NetAddress_t& outNetAddr, void *buffer, const size_t maxByteCount) {

	if (IsClosed()) {
		return 0U;
	}

	sockaddr_storage fromaddr; // who am I getting data from
	int addr_len = sizeof(sockaddr_storage);

	int recv = ::recvfrom(m_handle, (char*)buffer, (int)maxByteCount, 0, (sockaddr*)&fromaddr, &addr_len);

	if (recv > 0) {
		outNetAddr.FromSockaddr((sockaddr*)&fromaddr);

		return recv;
	}
	else {
		if (HasFatalError()) {
			Close();
		}
		return 0U;
	}
}
