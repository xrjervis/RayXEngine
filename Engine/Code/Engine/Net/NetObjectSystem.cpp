#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/core/Logger.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/NetObjectSystem.hpp"
#include "Engine/Net/NetObject.hpp"
#include "Engine/Net/NetSession.hpp"
#include "Engine/Net/NetObjectDefinition.hpp"

NetObjectSystem::NetObjectSystem() {

}

NetObjectSystem::~NetObjectSystem() {
	for (auto it : m_objectDefinitions) {
		delete it.second;
	}
}

void NetObjectSystem::RegisterDefinition(const NetObjectDefinition& defn) {
	NetObjectDefinition* objDef = new NetObjectDefinition(defn);
	m_objectDefinitions.insert({ objDef->m_id, objDef });
}

void NetObjectSystem::RegisterNetObject(NetObjectDefinition* defn, u16 networkID, void* localPtr) {
	std::unique_ptr<NetObject> newObject = std::make_unique<NetObject>();
	NetObject* netObj = newObject.get();
	netObj->m_defn = defn;
	netObj->m_networkID = networkID;
	netObj->m_localObj = localPtr;

	m_objects.push_back(std::move(newObject));
	m_idLookUp.insert({ networkID, netObj });
	m_localObjLookUp.insert({ localPtr, netObj });

	// Register NetobjectView
	NetObjectView netObjView;
	netObjView.m_lastSentTime = m_owningSession->GetNetTimeInSeconds();
	netObjView.m_netObj = netObj;

	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		NetObjectConnectionView* connView = m_connectionViews[i].get();
		if (connView) {
			m_connectionViews[i]->m_netObjViews.push_back(netObjView);
		}
	}
}

void NetObjectSystem::UnregisterNetObject(NetObject* netObj) {
	for (size_t i = 0; i < MAX_CONNECTIONS; ++i) {
		NetObjectConnectionView* connView = m_connectionViews[i].get();
		if (connView) {
			connView->DestroyNetObjectView(netObj);
		}
	}

	for(auto it = m_objects.begin(); it != m_objects.end(); ++it){
		if((*it).get() == netObj){
			u16 networkID = netObj->m_networkID;
			void* localObj = netObj->m_localObj;
			m_idLookUp.erase(networkID);
			m_localObjLookUp.erase(localObj);
			m_objects.erase(it);
			break;
		}
	}
}

void NetObjectSystem::SyncObject(u8 objId, void* ptr) {
	u16 networkID = FindAvailableNetworkID();
	NetObjectDefinition* defn = m_objectDefinitions.at(objId);

	RegisterNetObject(defn, networkID, ptr);

	NetMessageDefinition_t* msgDef = m_owningSession->GetMessageDefinitionByName("object_create");
	NetMessage createMsg(msgDef);

	createMsg.WriteBytes(&objId, 1);  //e.g. NETOBJ_PLAYER
	createMsg.WriteBytes(&networkID, 2); //e.g player's slot in m_objects;			

	defn->m_sendCreateFunc(createMsg, ptr);

	m_owningSession->SendToAllOthers(createMsg, CONNECTION_READY);
}

void NetObjectSystem::UnSyncObject(void* ptr) {
	NetObject* netObj = m_localObjLookUp.at(ptr);
	NetObjectDefinition* defn = netObj->m_defn;

	NetMessageDefinition_t* msgDef = m_owningSession->GetMessageDefinitionByName("object_destroy");
	NetMessage destroyMsg(msgDef);

	destroyMsg.WriteBytes(&netObj->m_networkID, 2);

	defn->m_sendDestroyFunc(destroyMsg, ptr);

	m_owningSession->SendToAllOthers(destroyMsg, CONNECTION_READY);

	UnregisterNetObject(netObj);
}

NetObjectDefinition* NetObjectSystem::GetNetObjectDefinition(u8 objID) const {
	return m_objectDefinitions.at(objID);
}

u16 NetObjectSystem::FindAvailableNetworkID() const {
	for(u16 id = 0; id < MAX_NETWORK_ID; ++id){
		bool flag = true;
		for(auto it : m_idLookUp){
			if(it.first == id){
				flag = false;
				break;
			}
		}
		if(flag){
			return id;
		}
	}
	ERROR_AND_DIE("Find available network id failed!");
	return 0u;
}

// void NetObjectSystem::SendConnectionViewSnapshots() {
// 	NetMessageDefinition_t* msgDef = m_owningSession->GetMessageDefinitionByName("object_update");
// 	NetMessage updateMsg(msgDef);
// 
// 	std::list<NetObjectView*> updates;
// 	if (!m_connectionViews.empty()) {
// 		for (auto& connectionView : m_connectionViews) {
// 			if (connectionView && !connectionView->m_netObjViews.empty()) {
// 				for (auto& netObjView : connectionView->m_netObjViews) {
// 					updates.push_back(&netObjView);
// 				}
// 			}
// 		}
// 	}
// 
// 	updates.sort([](NetObjectView* a, NetObjectView* b) { return a->m_lastSentTime < b->m_lastSentTime; });
// 
// 	while (!updates.empty()) {
// 		NetObjectView* netObjView = updates.front();
// 		NetObject* netObj = netObjView->m_netObj;
// 		NetObjectDefinition* defn = netObj->m_defn;
// 		u16 networkID = netObj->m_networkID;
// 		if (updateMsg.GetWritableByteCount() >= defn->m_snapshotSize + 3) {
// 			updateMsg.WriteBytes(&defn->m_id, 1);
// 			updateMsg.WriteBytes(&networkID, 2);
// 			defn->m_getSnapshotFunc(netObj->m_currentSnapshot, netObj->m_localObj);
// 			defn->m_sendSnapshotFunc(updateMsg, netObj->m_currentSnapshot);
// 
// 			netObjView->m_lastSentTime = m_owningSession->GetNetTimeInSeconds();
// 			updates.pop_front();
// 		}
// 		else {
// 			break;
// 		}
// 	}
// 
// 	m_owningSession->SendToAllOthers(updateMsg, CONNECTION_READY);
// }
