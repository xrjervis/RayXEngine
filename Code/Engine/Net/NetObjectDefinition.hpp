#pragma once
#include "Engine/Net/NetMessage.hpp"
#include <functional>


enum eNetObjectID : u8{
	NETOBJ_PLAYER,
	NETOBJ_BULLET,
	NUM_NETOBJ
};

class NetObjectDefinition {
public:
	NetObjectDefinition() = default;
	~NetObjectDefinition() = default;

public:
	eNetObjectID									m_id;
	std::function<void(NetMessage&, const void*)>	m_sendCreateFunc;
	std::function<void*(const NetMessage&)>			m_recvCreateFunc;

	std::function<void(NetMessage&, const void*)>	m_sendDestroyFunc;
	std::function<void(const NetMessage&, void*)>	m_recvDestroyFunc;

	size_t											m_snapshotSize;
	std::function<void(void*, const void*)>			m_getSnapshotFunc;
	std::function<void(NetMessage&, const void*)>	m_sendSnapshotFunc;
	std::function<void(const NetMessage&, void*)>	m_recvSnapshotFunc;
	std::function<void(void*, const void*, float)>  m_applySnapshotFunc;
};