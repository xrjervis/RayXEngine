#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/StringId.hpp"
#include "Engine/Net/NetAddress.hpp"

struct NetConnectionInfo_t {
	NetAddress_t address;
	StringId playerID;
	u8 index;
};

enum eConnectionState {
	CONNECTION_DISCONNECTED,
	CONNECTION_CONNECTED,
	CONNECTION_READY
};