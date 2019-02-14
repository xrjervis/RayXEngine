#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Profiler/ProfileScope.hpp"
#include "Engine/Profiler/ProfilerReport.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Camera.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/InputSystem/InputSystem.hpp"

#if defined(PROFILER_ENABLED)

static bool Command_Profiler(Command& cmd) {
	if (cmd.m_args.empty()) {
		ConsolePrintf("%s", cmd.GetName().c_str());
		g_theProfiler->ToggleOpen();
		return true;
	}
	else {
		return false;
	}
}

static bool Command_ProfilerPause(Command& cmd) {
	if (cmd.m_args.empty()) {
		ConsolePrintf("%s", cmd.GetName().c_str());
		g_theProfiler->Pause();
		return true;
	}
	else {
		return false;
	}
}

static bool Command_ProfilerResume(Command& cmd) {
	if (cmd.m_args.empty()) {
		ConsolePrintf("%s", cmd.GetName().c_str());
		g_theProfiler->Resume();
		return true;
	}
	else {
		return false;
	}
}

Profiler::Profiler() {
	Initialize();
}

Profiler::~Profiler() {

}

void Profiler::Initialize() {
	// Load profiler font


	// Profiler commands registration
	CommandDefinition::Register("profiler", "[N/A] Toggle profiler.", Command_Profiler);
	CommandDefinition::Register("profiler_pause", "[N/A] Pause profiler.", Command_ProfilerPause);
	CommandDefinition::Register("profiler_resume", "[N/A] Resume profiler.", Command_ProfilerResume);

	// Create mouse boxes
	m_sortByTotalBoxBound = AABB2(Vector2(960, 465), Vector2(1077, 484));
	m_sortBySelfBoxBound = AABB2(Vector2(1294, 465), Vector2(1407, 484));
	//m_dimension = Window::Get(0)->GetOrtho().GetDimensions();
	m_cpuInfoStartY = m_dimension.y * 0.70f;
	m_cpuVisualBoxStartX = m_dimension.x * 0.25f;
	m_cpuVisualBoxWidth = m_dimension.x * 0.6f;
	m_cpuVisualBoxHeight = m_dimension.y * 0.15f;
	m_cpuVisualBoxFrameDeltaX = m_cpuVisualBoxWidth / MAX_FRAMEHISTORY_COUNT;
	m_visualBoxBound = AABB2(Vector2(m_cpuVisualBoxStartX, m_cpuInfoStartY - m_cpuVisualBoxHeight), Vector2(m_cpuVisualBoxStartX + m_cpuVisualBoxWidth, m_cpuInfoStartY));
}

void Profiler::Push(const std::string& tag) {
#ifndef PROFILER_ENABLED
	return;
#endif
	if (IsPaused()) {
		return;
	}
	// adds a new leaf to the tree, and sets it as the active node.
	if (m_currentNode != nullptr) {
		auto newNode = std::make_unique<Profile_Node_t>(tag);
		newNode->SetStartHPC(GetPerformanceCounter());
		newNode->SetParent(m_currentNode);
		m_currentNode->AddChild(std::move(newNode));
		m_currentNode = m_currentNode->m_children.back().get();
	}
}

void Profiler::Pop() {
#ifndef PROFILER_ENABLED
	return;
#endif
	if (IsPaused()) {
		return;
	}
	// returns sets the active node to the current active node's parent. If the active node becomes nullptr, then a tree is considered complete;
	GUARANTEE_OR_DIE(m_currentNode != nullptr, "Profiler::Pop - Someone called pop without a push!");
	m_currentNode->SetEndHPC(GetPerformanceCounter());
	m_currentNode = m_currentNode->m_parent;
}

void Profiler::MarkFrame() {
#ifndef PROFILER_ENABLED
	return;
#endif
	if (m_currentNode != nullptr) {
		Pop();
		GUARANTEE_OR_DIE(m_currentNode == nullptr, "Profiler::MarkFrame - Someone forgot to pop!");
		// The trick here is you can't immediately pause profiling - you have to let the current frame finish. 
		// So pausing/resuming does not take affect until the next frame;
		if (m_isReadyToPause) {
			m_isPaused = true;
			m_isReadyToPause = false;
		}
	}

	if (IsPaused()) {
		if (m_isReadyToResume) {
			m_isPaused = false;
			m_isReadyToResume = false;
		}
		else {
			return;
		}
	}

	// create new node for this frame
	auto newRoot = std::make_unique<Profile_Node_t>("root");
	m_currentNode = newRoot.get();
	m_currentNode->SetStartHPC(GetPerformanceCounter());
	m_frames.push_back(std::move(newRoot));

	// check frame history length
	if (m_frames.size() > MAX_FRAMEHISTORY_COUNT) {
		m_frames.pop_front();
	}
}

void Profiler::Pause() {
	m_isReadyToPause = true;
	m_mouseSelectFrameIdx = -1;
}

void Profiler::Resume() {
	m_isReadyToResume = true;
	m_mouseSelectFrameIdx = -1;
}

void Profiler::ToggleOpen() {
	m_isOpen = !m_isOpen;
}

bool Profiler::IsPaused() const {
	return m_isPaused;
}

bool Profiler::IsOpen() const {
	return m_isOpen;
}

Profile_Node_t* Profiler::GetFrame(uint idx) const{
	return m_frames[idx].get();
}

u32 Profiler::GetFrameSize() const {
	return m_frames.size();
}

void Profiler::Update() {
	UpdateInput();

	if (!IsPaused()) {
		// generate report for current frame(s)
		auto& lastFrame = *(m_frames.end() - 2);
		m_report = std::make_unique<ProfilerReport>();
		if (m_isFlatView) {
			m_report->GenerateFlatView(lastFrame.get());
		}
		else {
			m_report->GenerateTreeView(lastFrame.get());
		}
	}
}

void Profiler::UpdateInput() {
	// Recalculate mouse position
	Vector2 mousePos = g_theInput->GetMouseClientPos();
	mousePos.y = m_dimension.y - mousePos.y;
	m_mousePos = mousePos;

	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_M)) {
		if (g_theInput->GetMouseMode() == MOUSEMODE_FREE) {
			g_theInput->SetMouseMode(MOUSEMODE_SNAP);
			m_isMouseHidden = true;
		}
		else {
			g_theInput->SetMouseMode(MOUSEMODE_FREE);
			m_isMouseHidden = false;
		}
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::KEYBOARD_V)) {
		m_isFlatView = !m_isFlatView;
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::MOUSE_LEFT)) {
		if (m_sortByTotalBoxBound.IsPointInside(m_mousePos)) {
			m_isSortByTotal = true;
		}
		if (m_sortBySelfBoxBound.IsPointInside(m_mousePos)) {
			m_isSortByTotal = false;
		}
		if (m_visualBoxBound.IsPointInside(m_mousePos)) {
			if (!IsPaused()) {
				Pause();
			}
			m_mouseSelectFrameIdx = (int)((m_mousePos.x - m_cpuVisualBoxStartX) / m_cpuVisualBoxFrameDeltaX);
			m_mouseSelectFrameIdx -= MAX_FRAMEHISTORY_COUNT - m_frames.size();
			if (m_mouseSelectFrameIdx >= 0) {
				m_report = std::make_unique<ProfilerReport>();
				if (m_isFlatView) {
					m_report->GenerateFlatView(GetFrame(m_mouseSelectFrameIdx));
				}
				else {
					m_report->GenerateTreeView(GetFrame(m_mouseSelectFrameIdx));
				}
			}
		}
	}
	if (g_theInput->WasKeyJustPressed(InputSystem::MOUSE_RIGHT)) {
		if (IsPaused()) {
			Resume();
		}
	}
}


void Profiler::Render() const {
	PROFILE_SCOPE_FUNTION();

/*	g_theRenderer->SetCamera(nullptr);*/
// 	g_theRenderer->SetShader(nullptr);
// 	g_theRenderer->SetTexture(0, nullptr);

	DrawGeneralInfo();

	DrawMemoryInfoText();

	DrawCPUInfoText();

	DrawCPUVIsualBox();

	DrawReportInfoText();

}

void Profiler::DrawReportInfoText() const {
	m_reportTextStartY = m_dimension.y * 0.5f;
	// Draw Report text
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_reportTextStartY), Vector2(0.f, 1.f),
// 		Stringf("%*s%-*s %-8s %-13s %-14s %-12s %-14s ",
// 			1, " ", 70, "FUNCTION NAME", "CALLS", "TOTAL TIME", "TOTAL %", "SELF TIME", "SELF %"),
// 		25.f, Vector3::Right, Vector3::Up, Rgba::GREEN);
	m_reportTextStartY -= 25.f;
	if (m_report) {
		if (m_isSortByTotal) {
			m_report->SortByTotalTime();
		}
		else {
			m_report->SortBySelfTime();
		}
		if (m_isFlatView) {
			// draw mouse hover background 
/*			g_theRenderer->SetTexture(0, nullptr);*/
			if (m_sortByTotalBoxBound.IsPointInside(m_mousePos)) {
//				g_theRenderer->DrawQuad(m_sortByTotalBoxBound.mins, Vector2::ZERO, m_sortByTotalBoxBound.GetDimensions().x, m_sortByTotalBoxBound.GetDimensions().y, AABB2(), Rgba(255, 255, 255, 150));
			}
			if (m_sortBySelfBoxBound.IsPointInside(m_mousePos)) {
//				g_theRenderer->DrawQuad(m_sortBySelfBoxBound.mins, Vector2::ZERO, m_sortBySelfBoxBound.GetDimensions().x, m_sortBySelfBoxBound.GetDimensions().y, AABB2(), Rgba(255, 255, 255, 150));
			}
			RenderFlatView();
		}
		else {
			// Tree view
			m_reportTreeViewData.clear();
			RenderTreeView(m_report->m_reportRoot.get());

			// Draw mouse select box
//			g_theRenderer->SetTexture(0, nullptr);
			for (auto& it : m_reportTreeViewData) {
				if (it.first.IsPointInside(m_mousePos)  && !m_isMouseHidden) {
//					g_theRenderer->DrawQuad(it.first.mins, Vector2::ZERO, it.first.GetDimensions().x, it.first.GetDimensions().y, AABB2(), Rgba(255, 255, 255, 150));
				}
			}
		}
	}
}

void Profiler::DrawMemoryInfoText() const {
//	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_dimension.y - 90.f), Vector2(0.f, 1.f), "Memory:", 40.f);
}

void Profiler::DrawCPUInfoText() const {
// 	// CPU info start
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_cpuInfoStartY), Vector2(0.f, 1.f), "CPU:", 40.f);
// 	double duration = GetFrame(m_frames.size() - 2)->GetDuration();
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_cpuInfoStartY - 40.f), Vector2(0.f, 0.5f), Stringf("  Last frame: %s", GetTimeString(duration).c_str()), 30.f);
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_cpuInfoStartY - 70.f), Vector2(0.f, 1.f), "  Live samples", 30.f);
}

void Profiler::DrawGeneralInfo() const {
// 	// Draw background color
// 	g_theRenderer->DrawQuad(Vector2::ZERO, Vector2::ZERO, m_dimension.x, m_dimension.y, AABB2(), Rgba(0, 0, 150, 200));
// 
// 	// Draw texts
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_dimension.y - 20.f), Vector2(0.f, 1.f), "PROFILER", 40.f);
// 	if (g_theInput->GetMouseMode() == MOUSEMODE_FREE) {
// 		g_theRenderer->DrawText(m_font.get(), Vector2(m_dimension.x * 0.3f, m_dimension.y - 20.f), Vector2(0.f, 1.f), "M - Mouse Visible", 40.f, Vector3::Right, Vector3::Up, Rgba::GREEN);
// 	}
// 	else {
// 		g_theRenderer->DrawText(m_font.get(), Vector2(m_dimension.x * 0.3f, m_dimension.y - 20.f), Vector2(0.f, 1.f), "M - Mouse Hidden", 40.f, Vector3::Right, Vector3::Up, Rgba::RED);
// 	}
// 	if (m_isFlatView) {
// 		g_theRenderer->DrawText(m_font.get(), Vector2(m_dimension.x * 0.6f, m_dimension.y - 20.f), Vector2(0.f, 1.f), "V - Flat View", 40.f, Vector3::Right, Vector3::Up, Rgba::WHITE);
// 	}
// 	else {
// 		g_theRenderer->DrawText(m_font.get(), Vector2(m_dimension.x * 0.6f, m_dimension.y - 20.f), Vector2(0.f, 1.f), "V - Tree View", 40.f, Vector3::Right, Vector3::Up, Rgba::WHITE);
// 
// 	}
// 	g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_dimension.y - 60.f), Vector2(0.f, 1.f), Stringf("FPS: %.2f", 1.f / Clock::GetMasterDeltaSeconds()), 30.f, Vector3::Right, Vector3::Up, Rgba::YELLOW);
}

void Profiler::DrawCPUVIsualBox() const {
// 	// Draw visual box
// 	g_theRenderer->SetTexture(0, nullptr);
// 	g_theRenderer->DrawLineBox(m_visualBoxBound, Rgba::WHITE);
// 	g_theRenderer->DrawQuad(Vector2(m_cpuVisualBoxStartX, m_cpuInfoStartY), Vector2(0.f, 1.f), m_cpuVisualBoxWidth, m_cpuVisualBoxHeight, AABB2(), Rgba(80, 80, 80, 200));
// 	MeshBuilder mb;
// 	mb.Begin(PRIMITIVE_TRIANGLE_STRIP, false);
// 	mb.SetUV(1.f, 1.f);
// 
// 	float maxTime = 0.020f;
// 	// frame size - 2, current frame is not completed yet
// 	for (int i = 0; i < m_frames.size() - 2; ++i) {
// 		float totalTime = (float)GetElapsedTime(m_frames[i]->m_startHPC, m_frames[i]->m_endHPC);
// 		if (totalTime > maxTime) {
// 			maxTime = totalTime;
// 		}
// 	}
// 	for (size_t i = 0; i < m_frames.size() - 1; ++i) {
// 		float currentFrameTime = (float)GetElapsedTime(m_frames[i]->m_startHPC, m_frames[i]->m_endHPC);
// 		float x = m_cpuVisualBoxStartX + m_cpuVisualBoxWidth - (float)(m_frames.size() - 2 - i) * m_cpuVisualBoxFrameDeltaX;
// 		float y_bottom = m_cpuInfoStartY - m_cpuVisualBoxHeight;
// 		float y_top = m_cpuInfoStartY;
// 		float y = RangeMapFloat(currentFrameTime, 0.01667f, maxTime, y_bottom, y_top);
// 		y = ClampFloat(y, y_bottom, y_top);
// 		Rgba color;
// 		if (currentFrameTime < 0.033f) {
// 			color = Rgba::GREEN;
// 		}
// 		else if (currentFrameTime < 0.044f) {
// 			color = Rgba::YELLOW;
// 		}
// 		else {
// 			color = Rgba::RED;
// 		}
// 		if (i == m_frames.size() - 2) {
// 			g_theRenderer->DrawText(m_font.get(), Vector2(m_cpuVisualBoxStartX + m_cpuVisualBoxWidth + 5.f, y), Vector2(0.f, 0.5f), GetTimeString(currentFrameTime).c_str(), 20.f, Vector3::Right, Vector3::Up, Rgba::YELLOW);
// 		}
// 
// 		mb.SetColor(color);
// 		mb.PushVertex(Vector2(x, y_bottom));
// 		mb.PushVertex(Vector2(x, y));
// 	}
// 	auto mesh = std::make_unique<Mesh>();
// 	mesh->TFromBuilder<Vertex_PCU>(mb);
// 	g_theRenderer->SetTexture(0, nullptr);
// 	g_theRenderer->DrawMesh(mesh.get());
// 
// 	// draw mouse selection line in visual box
// 	if (m_isMouseHidden == false && m_mouseSelectFrameIdx >= 0) {
// 		int relativeIdx = m_mouseSelectFrameIdx + MAX_FRAMEHISTORY_COUNT - m_frames.size();
// 		g_theRenderer->SetTexture(0, nullptr);
// 		g_theRenderer->DrawLine(Vector2((float)relativeIdx * m_cpuVisualBoxFrameDeltaX + m_cpuVisualBoxStartX, m_cpuInfoStartY - m_cpuVisualBoxHeight), Rgba::BLUE,
// 			Vector2((float)relativeIdx * m_cpuVisualBoxFrameDeltaX + m_cpuVisualBoxStartX, m_cpuInfoStartY), Rgba::BLUE, 1.f);
// 	}
}

void Profiler::RenderTreeView(ProfilerReport_Node_t* reportNode) const {
	UNUSED(reportNode);
// 	std::string funcName;
// 	if (reportNode->m_children.size() > 0) {
// 		if (reportNode->m_isCollapsed) {
// 			funcName = "[+]" + reportNode->m_name;
// 		}
// 		else {
// 			funcName = "[-]" + reportNode->m_name;
// 		}
// 	}
// 	else {
// 		funcName = "   " + reportNode->m_name;
// 	}
// 	reportNode->m_totalPercent = (float)(reportNode->m_totalTime / m_report->m_reportRoot->m_totalTime);
// 
// 	AABB2 textBox = g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_reportTextStartY), Vector2(0.f, 1.f),
// 		Stringf("%*s%-*s %-8llu %-12s %-14s %-12s %-14s ",
// 			reportNode->m_intent, " ",
// 			72 - reportNode->m_intent, funcName.c_str(),
// 			reportNode->m_callCount,
// 			GetTimeString(reportNode->m_totalTime).c_str(),
// 			Stringf("%.2f %%", reportNode->m_totalPercent * 100.f).c_str(),
// 			GetTimeString(reportNode->m_selfTime).c_str(),
// 			Stringf("%.2f %%", reportNode->m_selfPercent * 100.f).c_str()),
// 		25.f);
// 	m_reportTreeViewData[textBox] = reportNode;
// 	m_reportTextStartY -= 25.f;
// 	for (auto& child : reportNode->m_children) {
// 		RenderTreeView(child.second.get());
// 	}
}

void Profiler::RenderFlatView() const {
// 	for (auto& child : m_report->m_sortedChildren) {
// 		ProfilerReport_Node_t* report = child.second;
// 		report->m_totalPercent = (float)(report->m_totalTime / m_report->m_reportRoot->m_totalTime);
// 		g_theRenderer->DrawText(m_font.get(), Vector2(10.f, m_reportTextStartY), Vector2(0.f, 1.f),
// 			Stringf("%*s%-*s %-8llu %-12s %-14s %-12s %-14s ",
// 				1, " ",
// 				71, report->m_name.c_str(),
// 				report->m_callCount,
// 				GetTimeString(report->m_totalTime).c_str(),
// 				Stringf("%.2f %%", report->m_totalPercent * 100.f).c_str(),
// 				GetTimeString(report->m_selfTime).c_str(),
// 				Stringf("%.2f %%", report->m_selfPercent * 100.f).c_str()),
// 			25.f);
// 		m_reportTextStartY -= 25.f;
// 	}
}
#endif