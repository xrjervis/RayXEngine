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
#include "Engine/Core/DebugDrawSystem.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Game/TheApp.hpp"
#include "ThirdParty/pugixml/pugixml.hpp"

Uptr<TheApp> g_theApp;
Uptr<RHIOutput> g_mainOutput;
Uptr<Blackboard> g_gameConfig;

constexpr WCHAR* mainWindowTitle = L"SimpleMincraft";
constexpr float mainWindowAspect = 16.f / 9.f;
constexpr float mainWindowFractionToDesktop = 0.8f;
constexpr float mainWindowHeight = mainWindowFractionToDesktop * 1080.f;

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
		if (g_theApp->m_isDebugMode) {
			ConsolePrintf("Debug mode on!");
		}
		else {
			ConsolePrintf("Debug mode off!");
		}
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
		g_theApp->OnQuit();												return false;
	case WM_KEYDOWN:
		g_theInput->OnKeyPressed(keyCode);								return true;
	case WM_MBUTTONDOWN:
		g_theInput->OnMousePressed(InputSystem::MOUSE_MIDDLE);			return true;
	case WM_LBUTTONDOWN:
		g_theInput->OnMousePressed(InputSystem::MOUSE_LEFT);			return true;
	case WM_RBUTTONDOWN:
		g_theInput->OnMousePressed(InputSystem::MOUSE_RIGHT);			return true;
	case WM_KEYUP:
		g_theInput->OnKeyReleased(keyCode);								return true;
	case WM_MBUTTONUP:
		g_theInput->OnMouseReleased(InputSystem::MOUSE_MIDDLE);			return true;
	case WM_LBUTTONUP:
		g_theInput->OnMouseReleased(InputSystem::MOUSE_LEFT);			return true;
	case WM_RBUTTONUP:
		g_theInput->OnMouseReleased(InputSystem::MOUSE_RIGHT);			return true;
	default: return true;
	};
};


bool TestStandaloneFunction() {
	DebugString(8.f, "Test standalone function called!", Rgba::RED, Rgba::GREEN);
	return false;
}

TheApp::~TheApp() {
	g_theProfiler.reset();
	g_theAudio.reset();
	g_theConsole.reset();
	g_theInput.reset();
	g_theResourceManager.reset();
	g_theRHI.reset(); 
	g_theLogger.reset();
	g_theDebugDrawSystem.reset();
	g_theEventSystem.reset();
	g_theJobSystem.reset();
	g_theMasterClock.reset();
}

TheApp::TheApp() {
	// Test case
	NamedProperties p;
	p.Set<std::string>("FirstName", "Rui");
	p.Set("Age", 3);
	p.Set("FavoriteColor", Rgba::RED);

	std::string firstName = p.Get<std::string>("FirstName", "UNKOWN");
	int age = p.Get("Age", 0);
	Rgba color = p.Get("FavoriteColor", Rgba::BLACK);

	g_theMasterClock = std::make_unique<Clock>();
	g_theJobSystem = std::make_unique<JobSystem>();
	g_theEventSystem = std::make_unique<EventSystem>();
	g_theLogger = std::make_unique<Logger>();
	g_theRHI = std::make_unique<RHIInstance>();

	// Create the main window
	g_mainOutput = g_theRHI->GetDevice()->CreateOutput(mainWindowTitle, WINDOW_MODE_WINDOWED, mainWindowAspect);
	g_mainOutput->m_window->RegisterHandler(WinMsgHandler);
	
	g_theDebugDrawSystem = std::make_unique<DebugDrawSystem>();
	g_theResourceManager = std::make_unique<ResourceManager>();
	g_theInput = std::make_unique<InputSystem>();
	g_theConsole = std::make_unique<DevConsole>(g_mainOutput.get());
	g_theAudio = std::make_unique<AudioSystem>();
	g_theProfiler = std::make_unique<Profiler>();
	PostStartUp();
}

void TheApp::PostStartUp() {
	//--------------------------------------------------------------------
	// Test job
	g_theJobSystem->CreateWorkerThread("Alpha");
	g_theJobSystem->CreateWorkerThread("Beta");

	Uptr<TestJob> testJob1 = std::make_unique<TestJob>(5.f);
	Uptr<TestJob> testJob2 = std::make_unique<TestJob>(3.f);
	g_theJobSystem->QueueJob(std::move(testJob1));
	g_theJobSystem->QueueJob(std::move(testJob2));

	//--------------------------------------------------------------------

	m_masterClock = Clock::GetMasterClock();

	//Register commands
	RegisterDefaultCommands();

	//Run startup command script
	//g_theConsole->RunCommandFromFile("Data/startup.script");

	LoadDefaultAssets();

	//--------------------------------------------------------------------------
	// Load game config
	pugi::xml_document doc;
	doc.load_file("Data/GameConfig.xml");
	g_gameConfig = std::make_unique<Blackboard>();
	g_gameConfig->PopulateFromXmlElementAttributes(doc);

	m_game = std::make_unique<Game>();
}

void TheApp::RegisterDefaultCommands() {
	CommandDefinition::Register("quit", "[N/A] Exit the game.", Command_Quit);
	CommandDefinition::Register("get_stringid", "[\"string\"] Get StringId for a standard string.", Command_RegisterStringId);
	CommandDefinition::Register("debug_mode", "[N/A] Toggle debug mode.", Command_ToggleDebugMode);
	CommandDefinition::Register("log_test", "[int] Log Test. Argument is thread number that's going to be created.", Command_LogTest);
}

void TheApp::LoadDefaultAssets() {
	g_theResourceManager->LoadSampler("linear");
	g_theResourceManager->LoadSampler("nearest");
	g_theResourceManager->LoadTexture2D("default", "Data/Images/white.png");
	g_theResourceManager->LoadTexture2D("terrain_test", "Data/Images/terrain_test.png");
	g_theResourceManager->LoadTexture2D("terrain_diffuse", "Data/Images/terrain_diffuse.png");
	g_theResourceManager->LoadTexture2D("terrain_normal", "Data/Images/terrain_normal.png");
	g_theResourceManager->LoadTexture2D("test_opengl", "Data/Images/test_opengl.png");
	g_theResourceManager->LoadSpriteSheet("terrain_32x32", "Data/Images/terrain_32x32.png", IntVector2(32, 32));

	g_theResourceManager->LoadShaderProgram("default_vs", "Data/Shaders/hlsl/default_vs.hlsl", SHADER_TYPE_VERTEX_SHADER);
	g_theResourceManager->LoadShaderProgram("default_ps", "Data/Shaders/hlsl/default_ps.hlsl", SHADER_TYPE_PIXEL_SHADER);
	g_theResourceManager->LoadShaderProgram("smc_vs", "Data/Shaders/hlsl/smc_vs.hlsl", SHADER_TYPE_VERTEX_SHADER);
	g_theResourceManager->LoadShaderProgram("smc_ps", "Data/Shaders/hlsl/smc_ps.hlsl", SHADER_TYPE_PIXEL_SHADER);
	g_theResourceManager->LoadMaterial("default", "Data/Materials/default.mat");
	g_theResourceManager->LoadMaterial("smc", "Data/Materials/smc.mat");
	g_theResourceManager->LoadMaterial("debug", "Data/Materials/debug.mat");
	g_theResourceManager->LoadMaterial("wireframe", "Data/Materials/wireframe.mat");

}

void TheApp::RunFrame() {
	g_theProfiler->MarkFrame();
	g_theMasterClock->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
	Update();
	Render();
	g_theAudio->EndFrame();
	g_theInput->EndFrame();
}

void TheApp::Update() {
	g_theJobSystem->FinishAllCompletedJobs();

	UpdateDeltaTime();

	//-----------------------------------------------------------------
	// FPS Counter
	static float s_counter = 0.f;
	s_counter += m_deltaSeconds;
	if (s_counter >= 1.f) {
		s_counter = 0.f;
		std::string fpsStr = Stringf(" - FPS: %d(%.2fms)", (int)(1.f / m_deltaSeconds), m_deltaSeconds*1000.f);
		std::wstring fpsWstr(fpsStr.begin(), fpsStr.end());
		g_mainOutput->m_window->SetTitle(mainWindowTitle + fpsWstr);
	}

	if (g_theConsole->IsOpen()) {
		g_theConsole->Update(m_deltaSeconds);
	}
	else {
		UpdateInput();
	}

	m_game->Update();

	// Debug draw system
	g_theDebugDrawSystem->Update(m_deltaSeconds);

	// Profiler
	if (g_theProfiler->IsOpen() && g_theProfiler->GetFrameSize() > 1) {
		g_theProfiler->Update();
	}
}

void TheApp::Render() const {
	g_theRHI->GetImmediateRenderer()->BindOutput(g_mainOutput.get());
	g_theRHI->GetFontRenderer()->BindOutput(g_mainOutput.get());
	g_theRHI->GetFontRenderer()->SetFont(L"Fira Code");

	m_game->Render();
	
	// Debug draw system
	g_theDebugDrawSystem->Render3D();
	g_theDebugDrawSystem->Render2D();

	if (g_theProfiler->IsOpen() && g_theProfiler->GetFrameSize() > 1) {
		g_theProfiler->Render();
	}
	if (g_theConsole->IsOpen()) {
		g_theConsole->Render();
	}

	// Swap back buffer
	g_mainOutput->SwapBuffer();
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

bool TheApp::TestMemberMethod() {
	DebugString(3.f, "Test member method called!", Rgba::BLUE, Rgba::GREEN);
	return false;
}

void TheApp::UpdateInput() {
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE)) {
		OnQuit();
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_TILDE)) {
		g_theConsole->ToggleOpen();
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_E)) {
		g_theEventSystem->FireEvents("OnButtonHit", 4, 43.5f, Rgba::BLACK);
	}
}

TestJob::TestJob(int duration) 
	: m_liftTime(duration){
}

void TestJob::Execute() {
	::Sleep(m_liftTime * 1000);
}

void TestJob::JobCompleteCallback() {
	DebugString(3.f, Stringf("Job Completed: Sleep %d seconds!", m_liftTime), Rgba::RED, Rgba::RED);
}
