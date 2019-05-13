#pragma once
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Game/Game.hpp"
#include <vector>
#include <memory>

class RHIOutput;
class Clock;
class Blackboard;

class TestJob : public Job {
public:
	TestJob(int duration);
	virtual ~TestJob() = default;

	virtual void Execute() override;
	virtual void JobCompleteCallback() override;

private:
	int m_liftTime;
};

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
	bool						TestMemberMethod(const NamedProperties& args);

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