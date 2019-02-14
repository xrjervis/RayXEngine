#pragma once
#include <vector>
#include <array>
#include <map>
#include <memory>
#include "Engine/Net/NetObjectDefinition.hpp"
#include "Engine/Net/NetObjectConnectionView.hpp"

constexpr u16 MAX_NETWORK_ID = (u16)(-1);

class NetObject;
class NetSession;

class NetObjectSystem {
public:
	NetObjectSystem();
	~NetObjectSystem();

	void					RegisterDefinition(const NetObjectDefinition& defn);
	void					RegisterNetObject(NetObjectDefinition* defn, u16 networkID, void* localPtr);
	void					UnregisterNetObject(NetObject* netObj);
	void					SyncObject(u8 objId, void* ptr);
	void					UnSyncObject(void* ptr);
	NetObjectDefinition*	GetNetObjectDefinition(u8 objID) const;

	u16						FindAvailableNetworkID() const;

public:
	std::map<u8, NetObjectDefinition*>										m_objectDefinitions;
	std::vector<std::unique_ptr<NetObject>>									m_objects;
	std::map<u16, NetObject*>												m_idLookUp;
	std::map<void*, NetObject*>												m_localObjLookUp;
	std::array<std::unique_ptr<NetObjectConnectionView>, 16>				m_connectionViews;

	NetSession*																m_owningSession = nullptr;
};