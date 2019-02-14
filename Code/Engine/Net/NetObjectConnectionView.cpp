#include "Engine/Net/NetObjectConnectionView.hpp"
#include "Engine/Net/NetConnection.hpp"

void NetObjectConnectionView::DestroyNetObjectView(NetObject* netObj) {
	for (auto it = m_netObjViews.begin(); it != m_netObjViews.end(); ) {
		if ((*it).m_netObj == netObj) {
			it = m_netObjViews.erase(it);
		}
		else {
			++it;
		}
	}
}

