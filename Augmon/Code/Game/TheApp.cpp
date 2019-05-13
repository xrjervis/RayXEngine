#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Thread.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Game/TheApp.hpp"
#include "Game/EscapeGame.hpp"
#include "Game/ComposerGame.hpp"

std::unique_ptr<TheApp>		g_theApp;
std::unique_ptr<RHIOutput>	g_thiefOutput;
std::unique_ptr<RHIOutput>	g_boardOutput;


constexpr WCHAR* mainWindowTitle = L"Thief";
constexpr float mainWindowAspect = 16.f / 9.f;
constexpr float mainWindowFractionToDesktop = 0.8f;
constexpr float mainWindowHeight = mainWindowFractionToDesktop * 1080.f;

constexpr WCHAR* boardWindowTitle = L"Board";
constexpr float boardWindowAspect = 16.f / 9.f;
constexpr float boardWindowWidth = 1920.f;
constexpr float boardWindowHeight = 1080.f;

#pragma region Engine Commands
static bool Command_Quit(Command& cmd) {
	if (!cmd.m_args.empty()) {
		return false;
	}
	g_theApp->OnQuit();
	return true;
}

static bool Command_RegisterStringId(Command& cmd) {
	if (!cmd.m_args.empty()) {
		std::string str;
		cmd.GetNextArg<std::string>(str);
		StringId sid = CreateOrGetStringId(str);
		ConsolePrintf("%s %u", str.c_str(), sid);
		return true;
	}
	else {
		return false;
	}
}

static bool Command_ToggleDebugMode(Command& cmd) {
	if (!cmd.m_args.empty()) {
		return false;
	}
	else {
		g_theApp->m_isDebugMode = !g_theApp->m_isDebugMode;
		return true;
	}
}

static void LogTest() {
	// Open a file and output about 50MB of random numbers to it; 
	std::ifstream fin("Log/big.txt");
	if (!fin.is_open()) {
		return;
	}

	std::string line;
	while (std::getline(fin, line)) {
		if (g_theApp->IsQuitting()) {
			return;
		}
		LogPrintf("%s\n", line.c_str());
		LogErrorf("%s\n", line.c_str());
		LogWarningf("%s\n", line.c_str());
	}

	DebuggerPrintf("Finished ThreadTestWork");
}

static bool Command_LogTest(Command& cmd) {
	if (!cmd.m_args.empty()) {
		ConsolePrintf("%s", cmd.GetName().c_str());
		int threadCount;
		cmd.GetNextArg<int>(threadCount);
		if (WITHIN_RANGE(threadCount, 0, 4)) {
			for (int i = 0; i < threadCount; ++i) {
				threadHandle thread = g_theThreadManager.CreateThread(&LogTest);
				g_theThreadManager.Detach(thread);
			}
			return true;
		}
		else {
			ConsolePrintf("Please enter valid thread count: [0, 4]");
			return false;
		}
	}
	else {
		ConsolePrintf("Please enter valid thread count: [0, 4]");
		return false;
	}
}
#pragma endregion


bool WinMsgHandler(unsigned int msg, size_t wparam, size_t lparam) {
	UNUSED(lparam);
	unsigned char keyCode = (unsigned char)wparam;
	switch (msg) {
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		g_theApp->OnQuit();
		return false;
	case WM_KEYDOWN:
		g_theInput->OnKeyPressed(keyCode);
		return true;
	case WM_MBUTTONDOWN:
		g_theInput->OnMousePressed(1);
		return true;
	case WM_LBUTTONDOWN:
		g_theInput->OnMousePressed(0);
		return true;
	case WM_RBUTTONDOWN:
		g_theInput->OnMousePressed(2);
		return true;
	case WM_KEYUP:
		g_theInput->OnKeyReleased(keyCode);
		return true;
	case WM_MENURBUTTONUP:
		g_theInput->OnMouseReleased(1);
		return true;
	case WM_LBUTTONUP:
		g_theInput->OnMouseReleased(0);
		return true;
	case WM_RBUTTONUP:
		g_theInput->OnMouseReleased(2);
		return true;
	default: return true;
	};
};

TheApp::~TheApp() {
	// Stop the game first
	m_escapeGame.reset();

	g_theProfiler.reset();
	g_theAudio.reset();
	g_theConsole.reset();
	g_theInput.reset();
	g_theResourceManager.reset();
	g_theDebugDrawSystem.reset();
	g_theRHI.reset(); 
	g_theLogger.reset();
	g_theMasterClock.reset();
}

TheApp::TheApp() {
	g_theMasterClock = std::make_unique<Clock>();
	g_theLogger = std::make_unique<Logger>();
	g_theRHI = std::make_unique<RHIInstance>();

	// Create the second window
	g_boardOutput = g_theRHI->GetDevice()->CreateOutput(boardWindowTitle, WINDOW_MODE_BORDERLESS_FULLSCREEN, boardWindowAspect);
	g_boardOutput->m_window->AdjustPositionAndSize(Vector2(1920, 0), Vector2(1920, 1080));

	// Create the main window
	g_thiefOutput = g_theRHI->GetDevice()->CreateOutput(mainWindowTitle, WINDOW_MODE_WINDOWED, mainWindowAspect);
	g_thiefOutput->m_window->RegisterHandler(WinMsgHandler);

	g_theDebugDrawSystem = std::make_unique<DebugDrawSystem>();
	g_theResourceManager = std::make_unique<ResourceManager>();
	g_theInput = std::make_unique<InputSystem>();
	g_theConsole = std::make_unique<DevConsole>(g_thiefOutput.get());
	g_theAudio = std::make_unique<AudioSystem>();
	g_theProfiler = std::make_unique<Profiler>();

	PostStartUp();
}

void TheApp::PostStartUp() {
	g_theInput->SetMouseMode(MOUSEMODE_FREE);
	m_masterClock = Clock::GetMasterClock();

	//Register commands
	RegisterDefaultCommands();

	//Run startup command script
	//g_theConsole->RunCommandFromFile("Data/startup.script");

	//m_escapeGame = std::make_unique<EscapeGame>();
	m_composerGame = std::make_unique<ComposerGame>();
}

void TheApp::RegisterDefaultCommands() {
	CommandDefinition::Register("quit", "[N/A] Exit the game.", Command_Quit);
	CommandDefinition::Register("get_stringid", "[\"string\"] Get StringId for a standard string.", Command_RegisterStringId);
	CommandDefinition::Register("debug_mode", "[N/A] Toggle debug mode.", Command_ToggleDebugMode);
	CommandDefinition::Register("log_test", "[int] Log Test. Argument is thread number that's going to be created.", Command_LogTest);
}

void TheApp::RunFrame() {
	g_theProfiler->MarkFrame();
	PROFILE_SCOPE_FUNTION();

	g_theMasterClock->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	Update();
	Render();
	g_theAudio->EndFrame();
	g_theInput->EndFrame();
}

void TheApp::Update() {
	PROFILE_SCOPE_FUNTION();
	UpdateDeltaTime();

	if (g_theConsole->IsOpen()) {
		g_theConsole->Update(m_deltaSeconds);
	}
	else {
		UpdateInput();
	}

	//m_escapeGame->Update();
	m_composerGame->Update();

	// Debug draw system
	g_theDebugDrawSystem->Update(m_deltaSeconds);

	// Profiler
	if (g_theProfiler->IsOpen() && g_theProfiler->GetFrameSize() > 1) {
		g_theProfiler->Update();
	}
}

void TheApp::Render() const {
	PROFILE_SCOPE_FUNTION();
	//m_escapeGame->Render();
	m_composerGame->Render();

	// Debug draw system
	g_theRHI->GetFontRenderer()->BindOutput(g_boardOutput.get());
	g_theDebugDrawSystem->Render2D();

	if (g_theProfiler->IsOpen() && g_theProfiler->GetFrameSize() > 1) {
		g_theProfiler->Render();
	}
	if (g_theConsole->IsOpen()) {
		g_theConsole->Render();
	}

	// Swap back buffer
	g_thiefOutput->SwapBuffer();
	g_boardOutput->SwapBuffer();
}

void TheApp::UpdateDeltaTime() {
	m_deltaSeconds = Clock::GetMasterDeltaSeconds();
}

void TheApp::OnQuit() {
	m_isQuitting = true;
}

bool TheApp::IsQuitting() const {
	return m_isQuitting;
}

void TheApp::UpdateInput() {
	PROFILE_SCOPE_FUNTION();
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE)) {
		OnQuit();
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_TILDE)) {
		g_theConsole->ToggleOpen();
	}
}