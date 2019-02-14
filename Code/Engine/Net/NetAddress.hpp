#pragma once
#include "Engine/Core/WindowsCommon.hpp"
#include "Engine/Core/type.hpp"
#include <string>
#include <vector>



struct NetAddress_t {
public:
	NetAddress_t() = default;
	NetAddress_t(const sockaddr* addr);
	NetAddress_t(const std::string& str);
	~NetAddress_t() = default;

	bool ToSockaddr(sockaddr* out, size_t* out_addrlen) const;
	bool FromSockaddr(const sockaddr* addr);
	static std::string GetBindableAddress(u16 port);
	static bool GetAddressForHost(sockaddr* out, int* out_addrlen, const char* hostname, const char* service);
	static void GetAddressForAllLocal(std::vector<std::string>& addrs);
	std::string ToString() const;

public:
	u32 m_ipv4Addr = 0U;
	u16 m_port = 0U;
};