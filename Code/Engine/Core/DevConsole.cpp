#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/CommandDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileSystem.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"
#include "Engine/Renderer/d3d11/RHIOutput.hpp"
#include <sstream>

std::unique_ptr<DevConsole> g_theConsole;
constexpr int CONSOLE_STRINGF_LENGTH = 2048;
constexpr int DevConsole::MAX_HISTORY_STRINGS = 32;

DevConsole::DevConsole(RHIOutput* output)
	: m_output(output)
	, m_isOpen(false)
	, m_isSelecting(false)
	, m_curCursorIndex(0)
	, m_bottomLineIndex(0)
	, m_curHistoryIndex(-1)
	, m_selectStartIndex(-1)
	, m_selectEndIndex(-1)
	, m_curMatchingIndex(0)
	, m_curAutoSelectIndex(0)
	, m_isAutoCompleting(false) {
	m_stopWatch = std::make_unique<StopWatch>(g_theMasterClock.get());
	m_stopWatch->SetTimer(0.5f);

	RegisterCommands();

	SetFontSize(20.f);
	SetLineCount();

	//--------------------------------------------------------------------
	// Create devConsole 2d camera
	m_camera = std::make_unique<Camera>();
	m_camera->SetViewport(0U, 0, 0, m_output->GetWidth(), m_output->GetHeight(), 0.f, 1.f);
	m_camera->SetProjectionMode(ORTHOGRAPHIC);
	m_camera->SetOrtho(Vector2::ZERO, Vector2(m_output->GetWidth(), m_output->GetHeight()), 0.f, 10.f);

	m_camera->SetRenderTarget(m_output->GetRTV());
	m_camera->SetDepthTarget(m_output->GetDSV());
// 	g_theResourceManager->LoadSpriteSheet("morgana", L"Data/Images/morgana.png", IntVector2(4, 1));
// 	g_theResourceManager->LoadSpriteAnimation(L"Data/Animations/morgana.spriteanim");
//	m_spriteAnim = g_theResourceManager->GetSpriteAnimation("morgana_idle");
}

void DevConsole::SetLineCount() {
	m_lineCount = static_cast<int>((m_output->GetHeight() - (float)m_fontSize) / (float)m_fontSize);
}

void DevConsole::RegisterCommands() {
	CommandDefinition::Register("help", "List all known commands.", ConsoleHelp);
	CommandDefinition::Register("clear", "Clear history records.", ConsoleClear);
	CommandDefinition::Register("echo", "Prints the text with the color. echo (255,255,0,100) \"echo example\"", ConsoleEchoWithColor);
}

DevConsole::~DevConsole() {
}

void DevConsole::Update(float deltaSeconds) {
	UNUSED(deltaSeconds);
	PROFILE_SCOPE_FUNTION();

//	m_spriteAnim->Update(deltaSeconds);
	// pop type safe queue, and push data to output window
	ConsoleString cstr;
	while (m_queue.Dequeue(cstr)) {
		AppendOutputString(cstr);
	}

	UpdateInput();
}

void DevConsole::Render() {
	g_theRHI->GetImmediateRenderer()->BindOutput(m_output);
	g_theRHI->GetImmediateRenderer()->BindCamera(m_camera.get());

	g_theRHI->GetFontRenderer()->BindOutput(m_output);
	g_theRHI->GetImmediateRenderer()->BindMaterial(nullptr);

	g_theRHI->GetDevice()->ClearColor(m_output->GetRTV(), Rgba::MIDNIGHTBLUE);
	g_theRHI->GetDevice()->ClearDepthStencil(m_output->GetDSV(), 1.f, 0U);

	g_theRHI->GetFontRenderer()->SetFont(L"Consolas");
	g_theRHI->GetFontRenderer()->SetSize(m_fontSize);
	g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);

	RenderBackground();
	RenderInputBox();
	RenderCursor();
	RenderOutputBox();
	RenderAutoCompletionBox();
}

void DevConsole::RenderBackground() const {
//	g_theRHI->GetImmediateRenderer()->DrawQuad(m_output->GetCenter(), Vector2(0.5f, 0.5f), m_output->GetDimension(), Rgba::YELLOW);
}

void DevConsole::RenderCursor() const {
	static bool isDrawing = true;
	if(m_stopWatch->CheckAndReset()){
		if (isDrawing) {
			isDrawing = false;
		}
		else{
			isDrawing = true;
		}
	}

	if(isDrawing){
		AABB2 box(Vector2::ZERO, m_output->GetWidth(), GetFontSize());
		float startX = g_theRHI->GetFontRenderer()->GetTextWidth(">" + m_curInputString.substr(0, m_curCursorIndex), box);
		Vector2 startPos(startX, m_fontSize);
		Vector2 endPos(startX, 0.f);
		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos, endPos);
	}
	
}

void DevConsole::RenderInputBox() const {
	Vector2 startPos = Vector2::ZERO;
	Vector2 endPos(m_output->GetWidth(), GetFontSize());
	g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos + Vector2(0.f, GetFontSize()), endPos);
	g_theRHI->GetFontRenderer()->DrawTextInBox(">" + m_curInputString, AABB2(Vector2::ZERO, endPos));
	RenderSelectingTextBg();
}

void DevConsole::RenderSelectingTextBg() const {
	if (m_isSelecting) {
		AABB2 box(Vector2::ZERO, m_output->GetWidth(), GetFontSize());
		float startX = g_theRHI->GetFontRenderer()->GetTextWidth(">" + m_curInputString.substr(0, m_selectStartIndex), box);
		float endX = g_theRHI->GetFontRenderer()->GetTextWidth(">" + m_curInputString.substr(0, m_selectEndIndex), box);

		Vector2 startPos(startX, 0.f);
		Vector2 endPos(endX, GetFontSize());
		AABB2 selectBounds(startPos, endPos);
		g_theRHI->GetImmediateRenderer()->DrawQuad2D(
			selectBounds.GetCenter(),
			Vector2(.5f, .5f),
			selectBounds.GetDimensions(),
			Rgba(0.4f, 0.4f, 0.4f, 0.4f)
		);
	}
}

void DevConsole::RenderOutputBox() const {
	if(!m_curOutputStrings.empty()) {
		for (int i = 0; i < m_lineCount; ++i) {
			if (i >= m_curOutputStrings.size()) {
				break;
			}
			Vector2 start(0.f, (i + 1) * GetFontSize() + 3.f);
			Vector2 end(m_output->GetWidth(), (i + 2) * GetFontSize() + 3.f);
			const ConsoleString& cstr = m_curOutputStrings[m_curOutputStrings.size() - i - 1 - m_bottomLineIndex];
			g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_str, AABB2(start, end), cstr.m_color);
			if (cstr.m_cstrInfo) {
				start += Vector2((float)cstr.m_str.length() * (float)m_fontSize, 0.f);
				g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_cstrInfo->m_str, AABB2(start, end), cstr.m_cstrInfo->m_color);
			}
		}
		//Draw for scrolling visualization
		if (m_bottomLineIndex != 0) {
			std::string restLines = std::to_string(m_bottomLineIndex);
			Vector2 boxStart = Vector2(0.f, GetFontSize());
			g_theRHI->GetImmediateRenderer()->DrawQuad2D(
				boxStart,
				Vector2(0.f, 1.f),
				Vector2(m_output->GetWidth(), GetFontSize()),
				Rgba::GREEN
			);
			g_theRHI->GetFontRenderer()->DrawTextInBox(restLines + "...", AABB2(boxStart, Vector2(m_output->GetWidth(), 2.f * GetFontSize())), Rgba::RED);
		}
	}
}

void DevConsole::RenderAutoCompletionBox() const {
	if (m_isAutoCompleting) {
		float maxStrLength = 0.f;
		for (size_t i = 0; i < m_matchedStrings.size(); ++i) {
			const ConsoleString& cstr = m_matchedStrings[i];
			float strLength = g_theRHI->GetFontRenderer()->GetTextWidth(cstr.m_str + cstr.m_cstrInfo->m_str, AABB2(Vector2::ZERO, Vector2(m_output->GetWidth(), GetFontSize()))) + 100.f; //for a tab

			if (strLength > maxStrLength) {
				maxStrLength = strLength;
			}
		}
		Vector2 startPos(0.f, GetFontSize());
		g_theRHI->GetImmediateRenderer()->DrawQuad2D(
			startPos,
			Vector2(0.f, 1.f),
			Vector2(maxStrLength, (float)m_matchedStrings.size() * GetFontSize()),
			Rgba::BLACK
		);
		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos, startPos + Vector2(maxStrLength, 0.f));
		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos + Vector2(0.f, (float)m_matchedStrings.size() * m_fontSize),
			startPos + Vector2(maxStrLength, (float)m_matchedStrings.size() * m_fontSize));
		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos + Vector2(1.f, 0.f), startPos + Vector2(1.f, (float)m_matchedStrings.size() * m_fontSize));
		g_theRHI->GetImmediateRenderer()->DrawLine2D(startPos + Vector2(maxStrLength + 1.f, 0.f),
			startPos + Vector2(maxStrLength + 1.f, (float)m_matchedStrings.size() * m_fontSize));


		for (size_t i = 0; i < m_matchedStrings.size(); ++i) {
			const ConsoleString& cstr = m_matchedStrings[i];
			Vector2 start(0.f, ((int)i + 1) * m_fontSize);
			Vector2 end(maxStrLength, (float)(i + 2) * m_fontSize);
			AABB2 box(start, end);
			if (i == m_curAutoSelectIndex) {
				g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
				g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_str, box, Rgba::YELLOW);
				g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER);
				g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_cstrInfo->m_str, box, Rgba::GREEN);
			}
			else {
				g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
				g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_str, box, cstr.m_color);
				g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER);
				g_theRHI->GetFontRenderer()->DrawTextInBox(cstr.m_cstrInfo->m_str, box, cstr.m_cstrInfo->m_color);
			}
		}
		g_theRHI->GetFontRenderer()->SetAlignment(TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER);
	}
}

bool DevConsole::RunCommand(Command& cmd) {
	for (auto it = CommandDefinition::s_commandDefintions.begin(); it != CommandDefinition::s_commandDefintions.end(); ++it) {
		if (it->first == cmd.GetName()) {
			command_cb callbackFunction = it->second->m_callback;
			if (callbackFunction(cmd)) {
				return true;
			}
			else {
				ConsolePrintf(m_errorOutputColor, "%s: has invalid arguments!", cmd.GetName().c_str());
				return false;
			}
		}
	}
	ConsolePrintf(m_errorOutputColor, "%s: Invalid command!", cmd.GetName().c_str());
	return false;
}

void DevConsole::RunCommandFromFile(const char* fileName) {
	std::string buffer = FileSystem::FileReadToBuffer(fileName);
	std::istringstream bufferStream(buffer);
	std::string commandString;
	while (std::getline(bufferStream, commandString, '\n')) {
		Command cmd(commandString);
		RunCommand(cmd);
	}
}

void DevConsole::ToggleOpen() {
	m_isOpen = !m_isOpen;
	if (m_isOpen) {
		Open();
	}
	else {
// 		ClearOutputBox();
 		ClearInputBox();
 		ClearSearchResults();
		Close();
	}
}

void DevConsole::Close() {
	m_output->GetWindow()->UnregisterHandler(&DevConsole::ConsoleMsgHandler);
	DebuggerPrintf("Console closed!");
}

void DevConsole::Open() {
	m_output->GetWindow()->RegisterHandler(&DevConsole::ConsoleMsgHandler);
	DebuggerPrintf("Console opened!");
}

void DevConsole::Hook(console_printf_cb cb) {
	if (std::find(m_hooks.begin(), m_hooks.end(), cb) == m_hooks.end()) {
		m_hooks.push_back(cb);
	}
}

void DevConsole::UnHook(console_printf_cb cb) {
	m_hooks.erase(std::remove(m_hooks.begin(), m_hooks.end(), cb), m_hooks.end());
}

void DevConsole::AppendInputString(std::string str) {
	m_curInputString.insert(m_curCursorIndex, str);
	m_curCursorIndex += (int)str.length();
}

void DevConsole::AppendOutputString(const ConsoleString& cstr) {
	m_curOutputStrings.push_back(cstr);
}

void DevConsole::SetFontSize(float fontSize) {
	m_fontSize = fontSize;
}

void DevConsole::ClearInputBox() {
	m_curInputString = "";
	m_curCursorIndex = 0;
	m_curHistoryIndex = -1;
	m_bottomLineIndex = 0;
	ClearSelectingState();
	ClearSearchResults();
}

void DevConsole::ClearOutputBox() {
	m_curOutputStrings.clear();
}

void DevConsole::ClearHistoryRecord() {
	m_historyStrings.clear();
	m_curHistoryIndex = -1;
}

void DevConsole::RecordInputString() {
	m_historyStrings.push_back(m_curInputString);
	if (m_historyStrings.size() > MAX_HISTORY_STRINGS) {
		m_historyStrings.erase(m_historyStrings.begin());
	}
	ClearInputBox();
}

std::string DevConsole::ReadClipBoard() {
	OpenClipboard(0);
	HANDLE handle = GetClipboardData(CF_TEXT);
	char *data = (char*)GlobalLock(handle);
	std::string str(data);
	GlobalUnlock(handle);
	CloseClipboard();
	return str;
}

void DevConsole::WriteClipBoard(const std::string& str) {
	const char* output = str.c_str();
	const size_t len = str.length() + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}

bool DevConsole::ConsoleHelp(Command& cmd) {
	if (cmd.m_args.empty()) {
		for (auto& it : CommandDefinition::s_commandDefintions) {
			ConsolePrintf("%s -%s", it.first.c_str(), it.second->m_description.c_str());
		}
		return true;
	}
	else{
		return false;
	}
}

bool DevConsole::ConsoleClear(Command& cmd) {
	if (cmd.m_args.empty()) {
		ConsolePrintf("%s", cmd.GetName().c_str());
		g_theConsole->ClearInputBox();
		g_theConsole->ClearOutputBox();
		return true;
	}
	else {
		return false;
	}
}

bool DevConsole::ConsoleEchoWithColor(Command& cmd) {
	Rgba color;
	std::string echoString;

	if (!cmd.m_args.empty()) {
		cmd.GetNextArg<Rgba>(color);
		if (color.r == 1) {
			return false;
		}
	}
	else {
		return false;
	}
	if (!cmd.m_args.empty()) {
		cmd.GetNextArg<std::string>(echoString);
		if (echoString[0] != '\"') {
			return false;
		}
		else {
			while (!cmd.m_args.empty()) {
				std::string restStr;
				cmd.GetNextArg<std::string>(restStr);
				echoString += ' ' + restStr;
			}
			if (echoString[echoString.length() - 1] == '\"') {
				ConsolePrintf(color, "%s", echoString.c_str());
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		return false;
	}
}

bool DevConsole::ConsoleMsgHandler(unsigned int msg, size_t wparam, size_t lparam) {
	UNUSED(lparam);
	switch (msg) {
	case WM_CHAR:
		char inputChar = (char)wparam;
		if (inputChar >= 32 && inputChar <= 126) { //from ASCII table
			std::string str(1, inputChar);
			if (g_theConsole->m_isSelecting) {
				g_theConsole->DeleteSelectingText();
			}
			g_theConsole->AppendInputString(std::string(str.begin(), str.end()));
			SearchForAutoCompletion();
		}
	}
	return true;
}

void DevConsole::SearchForAutoCompletion() {
	g_theConsole->ClearSearchResults();
	for (auto& it : CommandDefinition::s_commandDefintions) {
		const std::string& strInSearch = it.first;
		std::string& inputStr = g_theConsole->m_curInputString;
		const std::string& subStrInSearch = strInSearch.substr(0, inputStr.length()); 
		if (subStrInSearch == inputStr) {
			g_theConsole->m_isAutoCompleting = true;

			ConsoleString result(strInSearch, Rgba::WHITE);
			result.m_cstrInfo = new ConsoleString(it.second->m_description, Rgba::WHITE);
			g_theConsole->AddSeachResult(result);
		}
	}
	if (g_theConsole->m_isAutoCompleting) {
		g_theConsole->m_isSelecting = true;
		g_theConsole->m_curInputString = g_theConsole->m_matchedStrings[0].m_str;
		g_theConsole->m_selectStartIndex = g_theConsole->m_curCursorIndex;
		g_theConsole->m_selectEndIndex = g_theConsole->m_curInputString.length();
		g_theConsole->m_curAutoSelectIndex = 0;
	}
}

void DevConsole::UpdateInput() {
	//Tilde ~/`
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_TILDE)) {
		ToggleOpen();
	}
	//Esc
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE)) {
		if (m_isAutoCompleting) {
			m_isAutoCompleting = false;
		}
		else {
			if (!m_curOutputStrings.empty() || !m_curInputString.empty()) {
				if (m_isSelecting) {
					ClearSelectingState();
				}
				else {
					ClearInputBox();
					ClearOutputBox();
				}
			}
			else {
				ToggleOpen();
			}
		}
	}
	//Enter/Return
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_ENTER)) {
		if (!m_curInputString.empty()) {
			Command cmd(m_curInputString);
			RecordInputString();
			RunCommand(cmd);
			ClearInputBox();
		}
	}
	//Backspace
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_BACKSPACE)) {
		ClearSearchResults();
		if (!m_curInputString.empty()) {
			if (m_isSelecting) {
				DeleteSelectingText();
			}
			else {
				if (m_curCursorIndex != 0) {
					m_curInputString.erase(m_curInputString.begin() + m_curCursorIndex - 1);
					m_curCursorIndex--;
				}
			}
		}
	}
	// Tab
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_TAB)) {
		m_isAutoCompleting = false;
		if (m_isSelecting) {
			m_curCursorIndex = m_selectEndIndex;
		}
		ClearSelectingState();
	}
	//Left arrow
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW)) {
		if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_SHIFT)) {
			m_curCursorIndex--;
			m_curCursorIndex = ClampInt(m_curCursorIndex, 0, m_curInputString.length());
			m_isSelecting = true;
			if (m_selectEndIndex == -1 && m_curCursorIndex > 0) {
				m_selectEndIndex = m_curCursorIndex + 1;
			}
			m_selectStartIndex = m_curCursorIndex;
		}
		else {
			m_isAutoCompleting = false;
			if (m_isSelecting) {
				m_curCursorIndex = m_selectStartIndex;
			}
			else {
				m_curCursorIndex--;
				m_curCursorIndex = ClampInt(m_curCursorIndex, 0, m_curInputString.length());
			}
			ClearSelectingState();
		}
	}
	//Right arrow
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW)) {
		if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_SHIFT)) {
			m_curCursorIndex++;
			m_curCursorIndex = ClampInt(m_curCursorIndex, 0, m_curInputString.length());
			m_isSelecting = true;
			if (m_selectStartIndex == -1 && m_curCursorIndex <= m_curInputString.length()) {
				m_selectStartIndex = m_curCursorIndex - 1;
			}
			m_selectEndIndex = m_curCursorIndex;
		}
		else {
			m_isAutoCompleting = false;
			if (m_isSelecting) {
				m_curCursorIndex = m_selectEndIndex;
			}
			else {
				m_curCursorIndex++;
				m_curCursorIndex = ClampInt(m_curCursorIndex, 0, m_curInputString.length());
			}
			ClearSelectingState();
		}
	}
	//Up arrow
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW)) {
		if (m_isAutoCompleting) {
			m_curAutoSelectIndex++;
			m_curAutoSelectIndex = ClampInt(m_curAutoSelectIndex, 0, (int)m_matchedStrings.size() - 1);
			g_theConsole->m_curInputString = g_theConsole->m_matchedStrings[m_curAutoSelectIndex].m_str;
			g_theConsole->m_selectStartIndex = g_theConsole->m_curCursorIndex;
			g_theConsole->m_selectEndIndex = g_theConsole->m_curInputString.length();
		}
		else {
			if (!m_historyStrings.empty()) {
				if (m_curHistoryIndex == -1) {
					m_curHistoryIndex = m_historyStrings.size() - 1;
				}
				else {
					m_curHistoryIndex--;
					if (m_curHistoryIndex < 0) {
						m_curHistoryIndex = m_historyStrings.size() - 1;
					}
				}
				m_curInputString = m_historyStrings[m_curHistoryIndex];
				m_curCursorIndex = m_curInputString.length();
			}
		}
	}
	//Down arrow
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW)) {
		if (m_isAutoCompleting) {
			m_curAutoSelectIndex--;
			m_curAutoSelectIndex = ClampInt(m_curAutoSelectIndex, 0, (int)m_matchedStrings.size() - 1);
			g_theConsole->m_curInputString = g_theConsole->m_matchedStrings[m_curAutoSelectIndex].m_str;
			g_theConsole->m_selectStartIndex = g_theConsole->m_curCursorIndex;
			g_theConsole->m_selectEndIndex = g_theConsole->m_curInputString.length();
		}
		else {
			if (!m_historyStrings.empty()) {
				if (m_curHistoryIndex == -1) {
					m_curHistoryIndex = 0;
				}
				else {
					m_curHistoryIndex++;
					if (m_curHistoryIndex >= m_historyStrings.size()) {
						m_curHistoryIndex = 0;
					}
				}
				m_curInputString = m_historyStrings[m_curHistoryIndex];
				m_curCursorIndex = m_curInputString.length();
			}
		}
	}
	//Delete
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_DELETE)) {
		if (m_isSelecting) {
			DeleteSelectingText();
		}
		else if ((int)m_curInputString.length() >= m_curCursorIndex) {
			m_curInputString.erase(m_curInputString.begin() + m_curCursorIndex);
		}
	}
	//Page up
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_PAGEUP)) {
		m_bottomLineIndex++;
		if (m_curOutputStrings.size() > m_lineCount) {
			m_bottomLineIndex = ClampInt(m_bottomLineIndex, 0, m_curOutputStrings.size() - m_lineCount);
		}
		else {
			m_bottomLineIndex = 0;
		}
	}
	//Page down
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_PAGEDOWN)) {
		m_bottomLineIndex--;
		if (m_curOutputStrings.size() > m_lineCount) {
			m_bottomLineIndex = ClampInt(m_bottomLineIndex, 0, m_curOutputStrings.size() - m_lineCount);
		}
		else {
			m_bottomLineIndex = 0;
		}
	}

	//Hold Control + x,c,v
	if (g_theInput->IsKeyPressed(InputSystem::KEYBOARD_CTRL)) {
		if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_X)) {
			if (m_isSelecting) {
				std::string str = m_curInputString.substr(m_selectStartIndex, m_selectEndIndex - m_selectStartIndex);
				WriteClipBoard(str);
			}
			DeleteSelectingText();
		}
		if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_C)) {
			if (m_isSelecting) {
				std::string str = m_curInputString.substr(m_selectStartIndex, m_selectEndIndex - m_selectStartIndex);
				WriteClipBoard(str);
			}
		}
		if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_V)) {
			if (m_isSelecting) {
				DeleteSelectingText();
			}
			std::string str = ReadClipBoard();
			if (str != "\r\n") {
				AppendInputString(str);
			}
		}
	}
}

void DevConsole::DeleteSelectingText() {
	if (!m_curInputString.empty() && m_isSelecting) {
		m_curInputString.erase(m_curInputString.begin() + m_selectStartIndex,
			m_curInputString.begin() + m_selectEndIndex);
		m_curCursorIndex = m_selectStartIndex;
	}
	ClearSelectingState();
}

void DevConsole::ClearSelectingState() {
	m_isSelecting = false;
	m_selectStartIndex = -1;
	m_selectEndIndex = -1;
}

void DevConsole::ClearSearchResults() {
	m_isAutoCompleting = false;
	m_curMatchingIndex = 0;
	m_matchedStrings.clear();
}

void DevConsole::AddSeachResult(const ConsoleString& matchedStr) {
	m_matchedStrings.push_back(matchedStr);
}

void ConsolePrintf(const char* format, ...) {
	if (g_theConsole == nullptr) {
		return;
	}
	char textLiteral[CONSOLE_STRINGF_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, CONSOLE_STRINGF_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[CONSOLE_STRINGF_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string str(textLiteral);
	ConsoleString cstr(str, g_theConsole->m_defaultOutputTextColor);
	g_theConsole->m_queue.Enqueue(std::move(cstr));
	
	for(auto& hook : g_theConsole->m_hooks){
		hook(str);
	}
}

void ConsolePrintf(const Rgba& color, const char* format, ...) {
	if(g_theConsole == nullptr){
		return;
	}
	char textLiteral[CONSOLE_STRINGF_LENGTH];
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	vsnprintf_s(textLiteral, CONSOLE_STRINGF_LENGTH, _TRUNCATE, format, variableArgumentList);
	va_end(variableArgumentList);
	textLiteral[CONSOLE_STRINGF_LENGTH - 1] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string str(textLiteral);
	ConsoleString cstr(str, color);
	g_theConsole->m_queue.Enqueue(std::move(cstr));

	for (auto& hook : g_theConsole->m_hooks) {
		hook(str);
	}
}