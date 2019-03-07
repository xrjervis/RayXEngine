#pragma once
#include "Engine/Core/CommandDefinition.hpp"
#include "Engine/Core/ThreadSafeContainer.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/struct.hpp"
#include <vector>
#include <memory>
#include <string>

typedef void(*console_printf_cb)(const std::string& data);

class StopWatch;
class RHIOutput;

class DevConsole {
public:
	DevConsole(RHIOutput* output);
	~DevConsole();

	void		Update(float deltaSeconds);
	void		Render();

	void		RenderBackground() const;
	void		RenderCursor() const;
	void		RenderInputBox() const;
	void		RenderSelectingTextBg() const;
	void		RenderOutputBox() const;
	void		RenderAutoCompletionBox() const;

	void		RegisterCommands();
	bool		RunCommand(Command& cmd);
	void		RunCommandFromFile(const char* fileName);

	void		ToggleOpen();
	void		Close();
	void		Open();
	void		Hook(console_printf_cb cb);
	void		UnHook(console_printf_cb cb);

	bool		IsOpen() const { return m_isOpen; }
	void		AppendInputString(std::string str);
	void		AppendOutputString(const ConsoleString& cstr);
	void		SetFontSize(float fontSize);
	float		GetFontSize() const { return m_fontSize; }
	void		SetLineCount();
	void		ClearInputBox();
	void		ClearOutputBox();
	void		ClearHistoryRecord();
	void		ClearSelectingState();
	void		ClearSearchResults();
	void		AddSeachResult(const ConsoleString& matchedStr);
	void		DeleteSelectingText();
	void		RecordInputString();

	static std::string ReadClipBoard();
	static void WriteClipBoard(const std::string& str);

	static bool ConsoleHelp(Command& cmd);
	static bool ConsoleClear(Command& cmd);
	static bool ConsoleEchoWithColor(Command& cmd);
	static bool ConsoleMsgHandler(unsigned int msg, size_t wparam, size_t lparam);

	static void SearchForAutoCompletion();

private:
	void		UpdateInput();

public:
	Rgba						m_defaultInputTextColor = Rgba::YELLOW;
	Rgba						m_defaultOutputTextColor = Rgba::GREEN;
	Rgba						m_errorOutputColor = Rgba::RED;
	static const int			MAX_HISTORY_STRINGS;
	ThreadSafeQueue<ConsoleString> m_queue;
	std::vector<console_printf_cb> m_hooks;

private:
	RHIOutput*					m_output = nullptr;
	std::unique_ptr<Camera> 	m_camera;
	bool						m_isOpen;
	bool						m_isSelecting;
	int							m_lineCount;
	int							m_bottomLineIndex;
	int							m_curCursorIndex;
	int							m_curHistoryIndex;
	int							m_selectStartIndex;
	int							m_selectEndIndex;
	int							m_curAutoSelectIndex;
	float						m_fontSize;
	std::string					m_curInputString;
	std::vector<ConsoleString>	m_curOutputStrings;
	std::vector<std::string>	m_historyStrings;

	std::vector<ConsoleString>  m_matchedStrings;
	int							m_curMatchingIndex;
	bool						m_isAutoCompleting;

	std::unique_ptr<StopWatch>	m_stopWatch;

//	SpriteAnimation*			m_spriteAnim = nullptr;
};

extern std::unique_ptr<DevConsole> g_theConsole;

void ConsolePrintf( const char* format, ... );
void ConsolePrintf( const Rgba& color, const char* format, ... );