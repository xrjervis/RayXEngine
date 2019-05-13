#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Time.hpp"
#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>

constexpr int MAX_FRAMEHISTORY_COUNT = 300;

struct ProfilerReport_Node_t;
class ProfilerReport;

struct Profile_Node_t {
	Profile_Node_t(const std::string& tag) :m_tag(tag) {}

	void AddChild(std::unique_ptr<Profile_Node_t> child) { m_children.emplace_back(std::move(child)); }
	void SetParent(Profile_Node_t* parent) { m_parent = parent; }
	void SetStartHPC(u64 hpc) { m_startHPC = hpc; }
	void SetEndHPC(u64 hpc) { m_endHPC = hpc; }
	double GetDuration() const { return GetElapsedTime(m_startHPC, m_endHPC); }

	std::string				m_tag;
	Profile_Node_t*			m_parent = nullptr;
	u64						m_startHPC = 0;
	u64						m_endHPC = 0;
	std::vector< std::unique_ptr<Profile_Node_t> >	m_children;
};

class Profiler {
public:
	Profiler();
	~Profiler();

	void			Initialize();
	void			Push(const std::string& tag);
	void			Pop();
	void			MarkFrame();
	void			Pause();
	void			Resume();
	void			ToggleOpen();
	bool			IsPaused() const;
	bool			IsOpen() const;
	Profile_Node_t* GetFrame(uint idx) const; // can also get any previous frame in history
	u32				GetFrameSize() const;

	void			Update();
	void			Render() const;

	void			DrawReportInfoText() const;
	void			DrawMemoryInfoText() const;
	void			DrawCPUInfoText() const;
	void			DrawGeneralInfo() const;
	void			DrawCPUVIsualBox() const;


private:
	void			UpdateInput();
	void			RenderTreeView(ProfilerReport_Node_t* reportNode) const;
	void			RenderFlatView() const;

private:
	std::deque<std::unique_ptr<Profile_Node_t>> m_frames;  //aka. history
	std::unique_ptr<ProfilerReport> m_report;

	Profile_Node_t*				m_currentNode = nullptr;
	bool						m_isPaused = false;
	bool						m_isReadyToPause = false;
	bool						m_isReadyToResume = false;
	bool						m_isOpen = false;
	bool						m_isFlatView = false;
	bool						m_isMouseHidden = true;
	// view data
	Vector2						m_mousePos;
	Vector2						m_dimension;


	mutable float				m_reportTextStartY;
	float						m_cpuInfoStartY;
	float						m_cpuVisualBoxStartX;
	float						m_cpuVisualBoxWidth;
	float						m_cpuVisualBoxHeight;
	float						m_cpuVisualBoxFrameDeltaX;
	int							m_mouseSelectFrameIdx = -1;
	AABB2						m_visualBoxBound;
	AABB2						m_sortBySelfBoxBound;
	AABB2						m_sortByTotalBoxBound;
	bool						m_isSortByTotal = false;

	mutable std::unordered_map<AABB2, ProfilerReport_Node_t*> m_reportTreeViewData;
};