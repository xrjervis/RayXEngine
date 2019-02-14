#include "Engine/Net/NetPacket.hpp"
#include "Engine/Net/NetSession.hpp"
#include "Engine/Core/Logger.hpp"

NetPacket::NetPacket() 
	: BytePacker(PACKET_MTU) {
}

NetPacket::NetPacket(void* buffer) 
	: BytePacker(PACKET_MTU, buffer) {
}

NetPacket::NetPacket(NetPacket&& other) 
	: BytePacker(std::move(other)) {
	m_sender = other.m_sender;
}

NetPacket& NetPacket::operator=(NetPacket&& other) {
	if(this != &other){
		BytePacker::operator=(std::move(other));
		m_sender = other.m_sender;
	}
	return *this;
}

NetPacket::~NetPacket() {

}

bool NetPacket::WriteHeader(const PacketHeader_t& header) {
	if(WriteBytes(&header.senderConnectionIdx, 1) == false){
		return false;
	}
	if (WriteBytes(&header.ack, 2) == false) {
		return false;
	}
	if (WriteBytes(&header.lastReceivedAck, 2) == false) {
		return false;
	}
	if (WriteBytes(&header.previousReceivedAckBitfield, 2) == false) {
		return false;
	}
	if (WriteBytes(&header.messageCount, 1) == false) {
		return false;
	}
	return true;
}

bool NetPacket::UpdateHeader(const PacketHeader_t& header) {
	if (WriteBytesAt(0, &header.senderConnectionIdx, 1) == false) {
		return false;
	}
	if (WriteBytesAt(1, &header.ack, 2) == false) {
		return false;
	}
	if (WriteBytesAt(3, &header.lastReceivedAck, 2) == false) {
		return false;
	}
	if (WriteBytesAt(5, &header.previousReceivedAckBitfield, 2) == false) {
		return false;
	}
	if (WriteBytesAt(7, &header.messageCount, 1) == false) {
		return false;
	}
	return true;
}

bool NetPacket::ReadHeader(PacketHeader_t& out) const{
	if(ReadBytes(&out.senderConnectionIdx, 1) == 0){
		return false;
	}
	if (ReadBytes(&out.ack, 2) == 0) {
		return false;
	}
	if (ReadBytes(&out.lastReceivedAck, 2) == 0) {
		return false;
	}
	if (ReadBytes(&out.previousReceivedAckBitfield, 2) == 0) {
		return false;
	}
	if (ReadBytes(&out.messageCount, 1) == 0) {
		return false;
	}
	return true;
}

bool NetPacket::WriteMessage(const NetMessage& msg) {
	size_t payloadLength = msg.GetWrittenByteCount();
	u8* buffer = (u8*)msg.GetBuffer();

	u16 totalLength = (u16)(payloadLength + msg.m_definition->GetHeaderSize());
	u8 messageIndex = msg.m_index;

	if(!WriteBytes(&totalLength, 2)){
		return false;
	}

	// When constructing a NetMessage, e.g. in Command_SentAdd, we only need to write the payload of a message
	// not the message header
	// But here, when constructing a NetPacket, we have to write the message header based on the option of a message
	// If it's unreliable, just write message index,
	// If it's reliable, we should add u16 (2 bytes) reliable id
	if(!WriteBytes(&messageIndex, 1)) {
		return false;
	}
	if(msg.m_definition->IsReliable()){
		if(!WriteBytes(&msg.m_reliableId, 2)){
			return false;
		}
	}
	if(!WriteBytes(buffer, payloadLength)){
		return false;
	}
	return true;
}

bool NetPacket::ReadMessage(NetMessage& out) const{
	u16 messageTotalLength;
	if(ReadBytes(&messageTotalLength, 2) != 2){
		return false;
	}

	u8 messageIndex;
	if(ReadBytes(&messageIndex, 1) != 1){
		return false;
	}
	out.m_index = messageIndex;

	size_t headerSize = 1;
	if(m_sender.session){
		NetMessageDefinition_t* msgDef = m_sender.session->GetMessageDefinitionByIndex(messageIndex);
		out.m_definition = msgDef;
		headerSize = msgDef->GetHeaderSize();
		if (headerSize > 1) {
			u16 reliableID;
			if (ReadBytes(&reliableID, 2) != 2) {
				return false;
			}
			out.m_reliableId = reliableID;
		}
	}

	
	size_t payloadLength = messageTotalLength - headerSize;


	u8* buffer = new u8[payloadLength];
	if(ReadBytes(buffer, payloadLength) != payloadLength){
		return false;
	}
	out.WriteBytes(buffer, payloadLength);

	delete[] buffer;
	return true;
}
