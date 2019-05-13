#pragma once
#include <string>
#include <functional>
#include "Engine/Core/BytePacker.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Net/NetMessageDefinition.hpp"

class NetMessage : public BytePacker {
public:
	NetMessage();
	NetMessage(NetMessageDefinition_t* def);
	virtual ~NetMessage();

public:
	NetMessageDefinition_t*		m_definition = nullptr;
	u8							m_index = INVALID_MESSAGE_INDEX;
	u16							m_reliableId = INVALID_RELIABLE_ID;
	float						m_resendTime = 0.f;
};
