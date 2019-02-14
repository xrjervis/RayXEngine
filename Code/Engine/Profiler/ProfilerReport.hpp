#pragma once
#include "Engine/Core/type.hpp"
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <functional>

struct Profile_Node_t;

struct ProfilerReport_Node_t {
	ProfilerReport_Node_t(const std::string& str) :m_name(str) {}
	void GetInfoFromFrame(Profile_Node_t* frame);
	void PopulateTree(Profile_Node_t* frame);
	void PopulateFlat(Profile_Node_t* frame);
	ProfilerReport_Node_t* CreateOrGetReportNode(const std::string& name);

	std::string m_name;
	u64 m_callCount = 0;
	double m_totalTime = 0.0;
	double m_selfTime = 0.0;
	float m_totalPercent = 1.f;
	float m_selfPercent = 1.f;
	double m_averageTimePerCall = 0.0;

	ProfilerReport_Node_t* m_parent = nullptr;
	std::map<std::string, std::unique_ptr<ProfilerReport_Node_t>> m_children;

	// view
	int m_intent = 1;
	bool m_isCollapsed = false;
};

class ProfilerReport {
public:
	ProfilerReport();
	~ProfilerReport();

	void GenerateTreeView(Profile_Node_t* root);
	void GenerateFlatView(Profile_Node_t* root);
	void SortBySelfTime();
	void SortByTotalTime();

public:
	std::unique_ptr<ProfilerReport_Node_t> m_reportRoot;
	std::map<double, ProfilerReport_Node_t*, std::greater<double>> m_sortedChildren;  //only for flat view
};