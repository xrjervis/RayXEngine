#pragma once
#include "Engine/Core/type.hpp"
#include <functional>

constexpr size_t MESSAGE_MTU = 1000;
constexpr u8 INVALID_MESSAGE_INDEX = 0xFF;
constexpr u16 INVALID_RELIABLE_ID = 0xFFFF;

enum eNetCoreMessage : u8 {
	NETCOREMSG_PING = 0,
	NETCOREMSG_PONG,
	NETCOREMSG_HEARTBEAT,
	NETCOREMSG_JOIN_REQUEST,
	NETCOREMSG_JOIN_DENY,
	NETCOREMSG_JOIN_ACCEPT,
	NETCOREMSG_NEW_CONNECTION,
	NETCOREMSG_JOIN_FINISHED,
	NETCOREMSG_UPDATE_CONN_STATE,
	NETCOREMSG_HANGUP,
	NETCOREMSG_SYNCTIME,

	NUM_NETCOREMSG
};

enum eNetGameMessage : u8 {
	NETGAMEMSG_TEST = NUM_NETCOREMSG,
	NETGAMEMSG_UNRELIABLE_TEST = 128,
	NETGAMEMSG_RELIABLE_TEST = 129,
	NETGAMEMSG_INORDER_TEST = 130,
	NETGAMEMSGCOUNT
};

enum eNetMessageOption : u32 {
	NETMSGOPTION_NONE = 0,
	NETMSGOPTION_CONNECTIONLESS = BIT_FLAG(0),
	NETMSGOPTION_RELIABLE = BIT_FLAG(1),
	NETMSGOPTION_INORDER = BIT_FLAG(2),
	NETMSGOPTION_HEARTBEAT = BIT_FLAG(3),
	NETMSGOPTION_RELIABLE_INORDER = NETMSGOPTION_RELIABLE | NETMSGOPTION_INORDER
};

class NetMessage;
struct NetSender_t;

using NetMessage_cb = std::function<bool(const NetMessage&, const NetSender_t&)>;

struct NetMessageDefinition_t {

	size_t GetHeaderSize() const {
		if(IsReliable()){
			return 3;
		}
		else{
			return 1;
		}
	}

	bool IsReliable() const {
		u32 result = (u32)options & (u32)NETMSGOPTION_RELIABLE;
		if (result == (u32)NETMSGOPTION_RELIABLE) {
			return true;
		}
		else {
			return false;
		}
	}

	bool IsInOrder() const {
		u32 result = (u32)options & (u32)NETMSGOPTION_RELIABLE_INORDER;
		if(result == (u32)NETMSGOPTION_RELIABLE_INORDER){
			return true;
		}
		else{
			return false;
		}
	}

	u8 index = INVALID_MESSAGE_INDEX;
	NetMessage_cb callback;
	eNetMessageOption options;
};