#include "Engine/Net/NetConnection.hpp"
#include "Engine/Net/NetSession.hpp"
#include "Engine/Net/NetObject.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <bitset>
#include <algorithm>

NetConnection::NetConnection() {

}

NetConnection::~NetConnection() {

}

void NetConnection::Send(const NetMessage& msg) {
	if (msg.m_definition->IsReliable()) {
		m_unsentReliables.push_back(msg);
	}
	else{
		m_unsentUnreliables.push(msg);
	}
}

void NetConnection::Flush() {
	NetPacket packet;

	PacketTracker_t* tracker = new PacketTracker_t();

	PacketHeader_t header;
	header.senderConnectionIdx = m_owningSession->m_connectionIndex;
	header.messageCount = 0U;
	header.ack = INVALID_PACKET_ACK;
	header.lastReceivedAck = m_lastReceivedAck;
	header.previousReceivedAckBitfield = m_previousReceivedAckBitfield;

	// the header is not finalized yet
	packet.WriteHeader(header); 

	// flush messages on net connection
	// 1. Unconfirmed reliables
	// 2. Unsent reliables
	// 3. Unsent unreliables
	if(!m_unconfirmedReliables.empty()){
		for (size_t i = 0; i < m_unconfirmedReliables.size(); ++i) {
			NetMessage* msg = &m_unconfirmedReliables[i];
			if(msg->m_resendTime >= 0.1f){ //100ms
				packet.WriteMessage(*msg);
				header.messageCount++; // increase message count in packet header
				tracker->TrackReliable(msg->m_reliableId);
				msg->m_resendTime = 0.f;
			}
		}
	}

	if(!m_unsentReliables.empty()){
		for (size_t i = 0; i < m_unsentReliables.size(); ++i) {
			if (tracker->reliableCount >= MAX_RELIABLES_PER_PACKET) {
				break;
			}
			NetMessage* msg = &m_unsentReliables[i];
			if(msg->m_reliableId == INVALID_RELIABLE_ID){
				msg->m_reliableId = GetNextReliableID();
			}
			packet.WriteMessage(*msg);
			header.messageCount++;
			tracker->TrackReliable(msg->m_reliableId);
			msg->m_resendTime = 0.f;

			m_unconfirmedReliables.push_back(*msg);
			m_unsentReliables.erase(m_unsentReliables.begin() + i);
			i--;
		}
	}

	while (!m_unsentUnreliables.empty()) {
		NetMessage msg = m_unsentUnreliables.front();
		m_unsentUnreliables.pop();

		packet.WriteMessage(msg);
		header.messageCount++;
	}

	// ---------------------- For NetObjectSystem -------------------------- //
	if(m_owningSession->IsHost() && m_owningSession->m_myConnection != this){
		NetObjectSystem* nos = m_owningSession->GetNetObjectSystem();

		std::list<NetObjectView*> updates;
		if (!nos->m_connectionViews.empty()) {
			NetObjectConnectionView* connectionView = nos->m_connectionViews[m_info.index].get();
			if (connectionView && !connectionView->m_netObjViews.empty()) {
				for (auto& netObjView : connectionView->m_netObjViews) {
					updates.push_back(&netObjView);
				}
			}
		}

		updates.sort([](NetObjectView* a, NetObjectView* b) { return a->m_lastSentTime > b->m_lastSentTime; });

		if (!updates.empty()) {

			size_t leftSpace = packet.GetWritableByteCount();

			while (!updates.empty()) {
				NetObjectView* netObjView = updates.front();
				NetObject* netObj = netObjView->m_netObj;
				NetObjectDefinition* defn = netObj->m_defn;
				u16 networkID = netObj->m_networkID;
				if (leftSpace >= defn->m_snapshotSize + 3) {
					leftSpace -= defn->m_snapshotSize + 3;

					NetMessageDefinition_t* msgDef = m_owningSession->GetMessageDefinitionByName("object_update");
					NetMessage updateMsg(msgDef);
					updateMsg.WriteBytes(&defn->m_id, 1);
					updateMsg.WriteBytes(&networkID, 2);
					// new off the snapshot
					netObj->m_currentSnapshot = new u8[defn->m_snapshotSize];
					defn->m_getSnapshotFunc(netObj->m_currentSnapshot, netObj->m_localObj);
					defn->m_sendSnapshotFunc(updateMsg, netObj->m_currentSnapshot);
					packet.WriteMessage(updateMsg);
					header.messageCount++;

					delete netObj->m_currentSnapshot;
					netObj->m_currentSnapshot = nullptr;

					netObjView->m_lastSentTime = m_owningSession->GetNetTimeInSeconds();
					updates.pop_front();
				}
				else {
					break;
				}
			}
		}
	}
	// --------------------------------------------------------------------- //


	// check if the packet is empty, don't send empty packets
	if(header.messageCount > 0U){
		// finalize header
		header.ack = GetNextAckToSend();
		packet.UpdateHeader(header);

		// reset last sent time
		m_lastSendTime = m_owningSession->GetNetTimeInSeconds();

		// add packet tracker to list
		tracker->ack = header.ack;
		AddPacketTracker(tracker);
		
		// send it off
		m_owningSession->m_socket->Send(m_info.address, packet.GetBuffer(), packet.GetWrittenByteCount());
	}
	else{
		delete tracker;
	}
}

void NetConnection::Receive(NetPacket* packet, PacketHeader_t* header) {
	// simulate loss
	if (CheckRandomChance(m_owningSession->m_lossChance) == true) {
		// packet is lost
/*		LogWarningf("packet simulate lost");*/
		return;
	}

	// simulate latency
	float currentSeconds = m_owningSession->GetNetTimeInSeconds();
	float simLatency = GetRandomFloatInRange(m_owningSession->m_minLatency, m_owningSession->m_maxLatency);
	currentSeconds += simLatency;

//	std::unique_lock<std::mutex> lock(m_rcvdLock);
	m_receivedPackets.push_back(std::make_tuple(currentSeconds, packet, header));
}

// Called by NetSession::ProcessPacketThreadWorker
void NetConnection::Process() {
	if (m_receivedPackets.empty()) {
		return;
	}

//	std::unique_lock<std::mutex> lock(m_rcvdLock);
	for (size_t i = 0; i < m_receivedPackets.size(); ++i) {
		float packetTimestamp = std::get<0>(m_receivedPackets[i]);

		if (packetTimestamp <= m_owningSession->GetNetTimeInSeconds()) {

			// meets old packet, process
			NetPacket* packet = std::get<1>(m_receivedPackets[i]);
			PacketHeader_t* header = std::get<2>(m_receivedPackets[i]);

			if(header->lastReceivedAck != INVALID_PACKET_ACK){
				// update last received time only when lastReceivedAck is valid
				m_lastReceivedTime = m_owningSession->GetNetTimeInSeconds();
				ConfirmPacket(header);
			}

			UpdateReceivedAcks(header);
			// process messages on packet
			// Read messages
			ProcessMessagesOnPacket(header, packet);

			// remove packet after processing
			delete packet;
			delete header;
			m_receivedPackets.erase(m_receivedPackets.begin() + i);
			--i;
		}

	}
}

void NetConnection::ProcessMessagesOnPacket(PacketHeader_t* header, NetPacket* packet) {
	for (u8 msgIdx = 0; msgIdx < header->messageCount; ++msgIdx) {
		// Construct am empty message ready for write data
		NetMessage* msg = new NetMessage();
		if (!packet->ReadMessage(*msg)) {
			LogErrorf("Receive %u garbage messages:", packet->GetWrittenByteCount());
			//for (size_t i = 0; i < packet->GetWrittenByteCount(); ++i) {
			//	LogErrorf("[0x%x]", ((u8*)packet->GetBuffer())[i]);
			//}
			return;
		}

		NetMessageDefinition_t* msgDef = msg->m_definition;
		// Execute NetMessage
		if (msgDef) {
			if (msgDef->IsInOrder()) {
				if (!HasReceivedReliableID(msg->m_reliableId)) {
					AddReceivedReliableID(msg->m_reliableId);
					AddOutOfOrderMessage(msg);
				}
				if (msg->m_reliableId == m_nextExpectedReliableId) {
					ProcessOutOfOrderMessages(&packet->m_sender);
				}
			}
			else if (msgDef->IsReliable()) {
				if (!HasReceivedReliableID(msg->m_reliableId)) {
					AddReceivedReliableID(msg->m_reliableId);
					msgDef->callback(*msg, packet->m_sender);
				}
				else {
				}
			}
			else {
				msgDef->callback(*msg, packet->m_sender);
				delete msg;
			}
		}
		else {
			delete msg;
			LogWarningf("Cannot process message: message definition is nullptr");
		}
	}
}

u16 NetConnection::GetNextAckToSend() {
	++m_nextSentAck;
	if(m_nextSentAck == INVALID_PACKET_ACK){
		++m_nextSentAck;
	}
	return m_nextSentAck;
}

u16 NetConnection::GetNextReliableID() {
	if(m_nextSentReliableId == INVALID_RELIABLE_ID){
		++m_nextSentReliableId;
	}
	return m_nextSentReliableId++;
}

void NetConnection::AddPacketTracker(PacketTracker_t* tracker) {
	tracker->sendTime = g_theMasterClock->GetCurrentSeconds();

	m_packetTrackerList.push_back(tracker);
	if(m_packetTrackerList.size() > 300){
		m_packetTrackerList.erase(m_packetTrackerList.begin());
	}

	m_totalSentPackets++;

	// compute loss
//	m_loss = (float)m_totalLostPackets / (float)m_totalSentPackets;
}

void NetConnection::ConfirmPacket(PacketHeader_t* header) {
	// remove packet trackers
	std::vector<u16> confirmedAcks;
	confirmedAcks.push_back(header->lastReceivedAck);
	// Calculate rtt
	for (auto tracker = m_packetTrackerList.begin(); tracker != m_packetTrackerList.end(); ++tracker) {
		if ((*tracker)->ack == header->lastReceivedAck) {
			// blend rtt
			float desiredRTT = g_theMasterClock->GetCurrentSeconds() - (*tracker)->sendTime;
			m_rtt = Interpolate(m_rtt, desiredRTT, 0.2f);
		}
	}
	for (u16 i = 0; i < 16; ++i) {
		u16 bitFlag = 1 << i;
		// if that bit is set
		if (header->previousReceivedAckBitfield & bitFlag) {
			confirmedAcks.push_back(header->lastReceivedAck - (i + 1U));
		}
	}
 
	for(auto confirmedAck : confirmedAcks){
		for(auto tracker = m_packetTrackerList.begin(); tracker != m_packetTrackerList.end(); ){

			if((*tracker)->ack == confirmedAck){
				// walk through all reliable ids related to the packet
				// and remove them from unconfirmed list
				for(size_t i = 0; i < (*tracker)->reliableCount; ++i){
					u16 reliableId = (*tracker)->reliableIDs[i];
					for(auto it = m_unconfirmedReliables.begin(); it != m_unconfirmedReliables.end();){
						if(it->m_reliableId == reliableId){
							it = m_unconfirmedReliables.erase(it);
						}
						else{
							++it;
						}
					}
				}
				// remove that tracker
				delete (*tracker);
				tracker = m_packetTrackerList.erase(tracker);
			}
			else{
				++tracker;
			}
		}
	}
}

void NetConnection::UpdateReceivedAcks(PacketHeader_t* header) {
	// v2 - Gaffer on games
	u16 receivedAck = header->ack;
	u16 dist = receivedAck - m_lastReceivedAck;
	if ((dist & 0x8000) == 0) {
		m_lastReceivedAck = receivedAck;
		// how do I update the bitfield?
		m_previousReceivedAckBitfield <<= dist; // giving self more safe (how much did I skip?)
		m_previousReceivedAckBitfield |= (1 << (dist - 1)); // set the old highests bit; 
	}
	else {
		// got an older ack than highest - which bit do we set?
		dist = m_lastReceivedAck - receivedAck; // dist from highest
		m_previousReceivedAckBitfield |= (1 << (dist - 1));  // set bit in history
	}
}

bool NetConnection::HasReceivedReliableID(u16 id) {
	for(u16 it : m_receivedReliableIDs){
		if(it == id){
			return true;
		}
	}
	return false;
}

void NetConnection::AddReceivedReliableID(u16 id) {
	m_receivedReliableIDs.push_back(id);
}

void NetConnection::AddOutOfOrderMessage(NetMessage* msg) {
	m_outOfOrderMessages.push_back(msg);
}

void NetConnection::ProcessOutOfOrderMessages(NetSender_t* sender) {
	for (auto it = m_outOfOrderMessages.begin(); it != m_outOfOrderMessages.end(); ) {
		NetMessage* msg = *it;
		if(msg->m_reliableId == m_nextExpectedReliableId){
			NetMessageDefinition_t* msgDef = msg->m_definition;
			msgDef->callback(*msg, *sender);
			m_nextExpectedReliableId++;
			m_outOfOrderMessages.erase(it);
			if(m_outOfOrderMessages.empty()){
				break;
			}
			else{
				it = m_outOfOrderMessages.begin();
			}
		}
		else{
			++it;
		}
	}
}
