#include <cassert>
#include <stdlib.h> 
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h> 
#include <ctime>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/TheApp.hpp"


//-----------------------------------------------------------------------------------------------
void Initialize() {
	srand(static_cast<unsigned int>(time(0)));
	g_theApp = std::make_unique<TheApp>();
}

//Destroy the global App instance
void Shutdown() {
	g_theApp.reset();
}

//-----------------------------------------------------------------------------------------------
int WinMain(HINSTANCE curInstance, HINSTANCE prevInstance, LPSTR commandLine, int numShowCmd) {
	UNUSED(curInstance);
	UNUSED(prevInstance);
	UNUSED(commandLine);
	UNUSED(numShowCmd);

	Initialize();
	while (!g_theApp->IsQuitting()) {
		g_theApp->RunFrame();
	}
	Shutdown();
	//_CrtDumpMemoryLeaks();
	return 0;
}