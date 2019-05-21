#pragma once
#include "Game/Game.hpp"
#include <vector>
#include <memory>

class RHIOutput;
class Clock;
class Blackboard;

class TheApp {
public:
	~TheApp();
	explicit TheApp();

	void						RunFrame();
	void						Update();
	void						UpdateDeltaTime();
	void						Render() const;
	void						OnQuit();
	bool						IsQuitting() const;

private:
	void						PostStartUp();
	void						RegisterDefaultCommands();
	void						LoadDefaultAssets();
	void						UpdateInput();

public:
	Clock*						m_masterClock = nullptr;
	std::unique_ptr<Game>		m_game;
	bool						m_isDebugMode = false;

private:
	bool						m_isQuitting = false;
	float						m_deltaSeconds;
};

extern Uptr<TheApp>				g_theApp;
extern Uptr<RHIOutput>			g_mainOutput;
extern Uptr<Blackboard>			g_gameConfig;