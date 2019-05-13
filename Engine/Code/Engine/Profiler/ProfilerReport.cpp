#include "Engine/Profiler/ProfilerReport.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Core/Time.hpp"
#include <algorithm>

void ProfilerReport_Node_t::GetInfoFromFrame(Profile_Node_t* frame) {
	m_callCount++;
	m_totalTime += GetElapsedTime(frame->m_startHPC, frame->m_endHPC);
	m_selfTime = m_totalTime;
	for (auto& child : frame->m_children) {
		m_selfTime -= GetElapsedTime(child->m_startHPC, child->m_endHPC);
	}
	auto tempNode  = this;
	while (tempNode->m_parent != nullptr) {
		tempNode = tempNode->m_parent;
	}
	m_selfPercent = (float)(m_selfTime / tempNode->m_totalTime);
}

void ProfilerReport_Node_t::PopulateTree(Profile_Node_t* frame) {
	GetInfoFromFrame(frame);
	for (auto& child : frame->m_children) {
		ProfilerReport_Node_t* reportNode = CreateOrGetReportNode(child->m_tag);
		reportNode->m_intent = m_intent + 1;
		reportNode->PopulateTree(child.get());
	}
}


void ProfilerReport_Node_t::PopulateFlat(Profile_Node_t* frame) {
	for (auto& child : frame->m_children) {
		ProfilerReport_Node_t* reportNode = CreateOrGetReportNode(child->m_tag);
		ProfilerReport_Node_t tempNode(child->m_tag);
		tempNode.GetInfoFromFrame(child.get());
		reportNode->m_callCount += tempNode.m_callCount;
		reportNode->m_totalTime += tempNode.m_totalTime;
		reportNode->m_selfTime += tempNode.m_selfTime;
		reportNode->m_selfPercent = (float)(reportNode->m_selfTime / GetElapsedTime(frame->m_startHPC, frame->m_endHPC));
		PopulateFlat(child.get());
	}
}

ProfilerReport_Node_t* ProfilerReport_Node_t::CreateOrGetReportNode(const std::string& name) {
	if (m_children.find(name) != m_children.end()) {
		return m_children[name].get();
	}
	else {
		auto reportNode = std::make_unique<ProfilerReport_Node_t>(name);
		reportNode->m_parent = this;
		m_children[name] = std::move(reportNode);
		return m_children[name].get();
	}
}

ProfilerReport::ProfilerReport() {

}

ProfilerReport::~ProfilerReport() {

}

void ProfilerReport::GenerateTreeView(Profile_Node_t* root) {
	m_reportRoot = std::make_unique<ProfilerReport_Node_t>(root->m_tag);
	m_reportRoot->PopulateTree(root);
}

void ProfilerReport::GenerateFlatView(Profile_Node_t* root) {
	m_reportRoot = std::make_unique<ProfilerReport_Node_t>(root->m_tag);
	m_reportRoot->PopulateFlat(root);
	m_reportRoot->GetInfoFromFrame(root);
}

void ProfilerReport::SortBySelfTime() {
	m_sortedChildren.clear();
	for (auto& child : m_reportRoot->m_children) {
		m_sortedChildren[child.second->m_selfTime] = child.second.get();
	}
	
}

void ProfilerReport::SortByTotalTime() {
	m_sortedChildren.clear();
	for (auto& child : m_reportRoot->m_children) {
		m_sortedChildren[child.second->m_totalTime] = child.second.get();
	}
}