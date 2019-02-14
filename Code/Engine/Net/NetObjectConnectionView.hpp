#pragma once

#include <list>
#include "Engine/Net/NetObjectView.hpp"

class NetObject;

class NetObjectConnectionView {
public:
	NetObjectConnectionView() {}
	~NetObjectConnectionView() {}

	void DestroyNetObjectView(NetObject* netObj);

public:
	std::list<NetObjectView> m_netObjViews;
};
