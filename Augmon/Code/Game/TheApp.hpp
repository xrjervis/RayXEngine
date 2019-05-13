#pragma once
#include <memory>

class RHIOutput;
class Clock;
class EscapeGame;
class ComposerGame;

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
	void						UpdateInput();

public:
	Clock*						m_masterClock = nullptr;
	Uptr<EscapeGame>			m_escapeGame;
	Uptr<ComposerGame>			m_composerGame;
	bool						m_isDebugMode = false;

private:
	bool						m_isQuitting = false;
	float						m_deltaSeconds;
};

extern std::unique_ptr<TheApp>		g_theApp;
extern std::unique_ptr<RHIOutput>	g_thiefOutput;
extern std::unique_ptr<RHIOutput>	g_boardOutput;
