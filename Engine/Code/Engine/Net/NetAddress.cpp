#include "Engine/Net/NetAddress.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Logger.hpp"
#include <sstream>

NetAddress_t::NetAddress_t(const std::string& str) {
	std::istringstream is(str);

	std::string ip;
	std::string port;
	std::getline(is, ip, ':');
	std::getline(is, port);

	sockaddr_storage saddr;
	int addrlen;
	NetAddress_t::GetAddressForHost((sockaddr*)&saddr, &addrlen, ip.c_str(), port.c_str());
	FromSockaddr((sockaddr*)&saddr);
}

NetAddress_t::NetAddress_t(const sockaddr* addr) {
	FromSockaddr(addr);
}

bool NetAddress_t::ToSockaddr(sockaddr* out, size_t* out_addrlen) const {
	*out_addrlen = sizeof(sockaddr_in);

	sockaddr_in* ipv4 = (sockaddr_in*)out;
	memset(ipv4, 0, sizeof(sockaddr_in));

	ipv4->sin_family = AF_INET;
	ipv4->sin_addr.S_un.S_addr = m_ipv4Addr;
	ipv4->sin_port = ::htons(m_port);
	return true;
}

bool NetAddress_t::FromSockaddr(const sockaddr* addr) {
	if (addr->sa_family != AF_INET) {
		return false;
	}
	// if IPv6 - also check AF_INET6
	const sockaddr_in* ipv4 = (const sockaddr_in*)addr;
	uint ip = ipv4->sin_addr.S_un.S_addr;
	u16 port = ::ntohs(ipv4->sin_port);

	m_ipv4Addr = ip;
	m_port = port;
	return true;
}

std::string NetAddress_t::GetBindableAddress(u16 port) {
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	char my_name[256];
	::gethostname(my_name, 256);
	std::string portStr = std::to_string(port);

	addrinfo* result = nullptr;
	int status = ::getaddrinfo(my_name, portStr.c_str(), &hints, &result);
	if (status != 0) {
		LogErrorf("Failed to find addresses for port[%s].  Error[%s]", portStr.c_str(), ::gai_strerror(status));
		return "";
	}

	std::vector<std::string> addrs;
	addrinfo* iter = result;

	while (iter != nullptr) {
		if (iter->ai_family == AF_INET) {
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr);
			char out[256];
			::inet_ntop(ipv4->sin_family, &(ipv4->sin_addr), out, 256);
			addrs.push_back(std::string(out));
		}
		iter = iter->ai_next;
	}
	::freeaddrinfo(result);
	return addrs[addrs.size() - 1] + ":" + portStr;
}

bool NetAddress_t::GetAddressForHost(sockaddr* out, int* out_addrlen, const char* hostname, const char* service) {
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	// check if my_name is null or empty
	hints.ai_family = AF_INET;			// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;	// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;		// and address we can host on;
	//hints.ai_flags |= AI_NUMERICHOST;
	addrinfo* result = nullptr;
	int status = ::getaddrinfo(hostname, service, &hints, &result);
	if (status != 0) {
		LogErrorf("Failed to find address for [%s:%s], Error[%s]", hostname, service, ::gai_strerror(status));
		return true;
	}

	addrinfo *iter = result;
	bool found_one = false;
	while (iter != nullptr) {
		if (iter->ai_family == AF_INET) {
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr);
			memcpy(out, ipv4, sizeof(sockaddr_in));
			*out_addrlen = sizeof(sockaddr_in);
			break;
		}
		iter = iter->ai_next;
	}
	::freeaddrinfo(result);
	return found_one;
}

void NetAddress_t::GetAddressForAllLocal(std::vector<std::string>& addrs) {
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// first, get the name of my machine
	char my_name[256];
	::gethostname(my_name, 256);
	const char* service = "80";

	addrinfo* result = nullptr;
	int status = ::getaddrinfo(my_name, service, &hints, &result);
	if (status != 0) {
		LogErrorf("Failed to find addresses for [%s:%s].  Error[%s]", my_name, service, ::gai_strerror(status));
		return;
	}

	addrinfo* iter = result;
	while (iter != nullptr) {
		if (iter->ai_family == AF_INET) {
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr);
			char out[256];
			::inet_ntop(ipv4->sin_family, &(ipv4->sin_addr), out, 256);
			addrs.push_back(std::string(out));
		}
		iter = iter->ai_next;
	}
	::freeaddrinfo(result);
}

std::string NetAddress_t::ToString() const {
	u8* array = (u8*)&m_ipv4Addr;
	return Stringf("%u.%u.%u.%u:%u", array[0], array[1], array[2], array[3], m_port);
}