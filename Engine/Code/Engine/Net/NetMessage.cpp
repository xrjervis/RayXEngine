#include "Engine/Net/NetMessage.hpp"

NetMessage::NetMessage(NetMessageDefinition_t* def)
	: BytePacker(MESSAGE_MTU) 
	, m_index(def->index) 
	, m_definition(def) {
	SetReadableByteCount(MESSAGE_MTU);
}


NetMessage::NetMessage() 
	: BytePacker(MESSAGE_MTU) {
	SetReadableByteCount(MESSAGE_MTU);
}

NetMessage::~NetMessage() {

}

