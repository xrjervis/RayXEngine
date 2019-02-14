#pragma once
#include "Engine/Core/type.hpp"

class NetObjectDefinition;

class NetObject{
public:
	NetObject() {}
	~NetObject() {}

public:
	u8*						m_currentSnapshot = nullptr;
	NetObjectDefinition*	m_defn = nullptr;
	u16						m_networkID = 0u;
	void*					m_localObj = nullptr;
};