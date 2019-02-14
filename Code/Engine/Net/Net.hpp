#pragma once
#pragma comment(lib, "ws2_32.lib") // winsock libraries
#include <memory>

class Net {
public:
	Net() { StartUp(); }
	~Net() { Shutdown(); }
	bool StartUp();
	void Shutdown();

	bool m_isRunning = false;
};

extern std::unique_ptr<Net> g_theNet;
