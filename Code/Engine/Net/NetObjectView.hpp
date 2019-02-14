#pragma once

class NetObject;

class NetObjectView{
public:
	NetObjectView() {}
	~NetObjectView() {}

public:
	float m_lastSentTime = 0.f;
	NetObject* m_netObj = nullptr;
};