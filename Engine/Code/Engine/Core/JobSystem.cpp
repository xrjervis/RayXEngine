#include "Engine/Core/JobSystem.hpp"

std::unique_ptr<JobSystem> g_theJobSystem;

int Job::s_globalID = 0;

JobSystem::JobSystem() {
}

JobSystem::~JobSystem() {
	for (auto& it = m_workerThreads.begin(); it != m_workerThreads.end(); ) {
		it->m_isRunning = false;
		it = m_workerThreads.erase(it);
	}
}

void JobSystem::FinishAllCompletedJobs() {
	std::lock_guard<std::mutex> lock(m_completedJobsMutex);
	for (auto& it = m_completedJobs.begin(); it != m_completedJobs.end(); ) {
		(*it)->JobCompleteCallback();
		it = m_completedJobs.erase(it);
	}
}

void JobSystem::CreateWorkerThread(const std::string& name, u32 channels /*= 0xFFFFFFFF*/) {
	m_workerThreads.emplace_back(name, channels);
}

void JobSystem::DestroyWorkerThread(const std::string& name) {
	for (auto& it = m_workerThreads.begin(); it != m_workerThreads.end(); ) {
		if (it->m_name == name) {
			it->m_isRunning = false;
			g_theThreadManager.Join(it->m_threadHandle);
			it = m_workerThreads.erase(it);
		}
		else {
			++it;
		}
	}
}

void JobSystem::QueueJob(Uptr<Job> job) {
	std::lock_guard<std::mutex> lock(m_queuedJobsMutex);
	m_queuedJobs.push_back(std::move(job));
}

Job* JobSystem::ClaimJob(u32 channels) {
	std::lock_guard<std::mutex> lock1(m_queuedJobsMutex);
	std::lock_guard<std::mutex> lock2(m_runningJobsMutex);
	for (auto& it = m_queuedJobs.begin(); it != m_queuedJobs.end(); ) {
		if ((*it)->m_channels == channels) {
			Job* j = (*it).get();
			m_runningJobs.push_back(std::move(*it));
			it = m_queuedJobs.erase(it);
			return j;
		}
		else {
			++it;
		}
	}
	return nullptr;
}

void JobSystem::PostJob(int jobID) {
	std::lock_guard<std::mutex> lock1(m_runningJobsMutex);
	std::lock_guard<std::mutex> lock2(m_completedJobsMutex);
	for (auto& it = m_runningJobs.begin(); it != m_runningJobs.end(); ) {
		if ((*it)->m_id == jobID) {
			m_completedJobs.push_back(std::move(*it));
			it = m_runningJobs.erase(it);
			return;
		}
		else {
			++it;
		}
	}
}

JobWorkerThread::JobWorkerThread(const std::string& name, u32 channels) 
	: m_name(name)
	, m_channels(channels) {
	m_threadHandle = g_theThreadManager.CreateThread(&JobWorkerThread::JobWorkerThreadEntryFunction, this);
	g_theThreadManager.Detach(m_threadHandle);
}

JobWorkerThread::~JobWorkerThread() {
}

void JobWorkerThread::JobWorkerThreadEntryFunction() {
	while (IsRunning()) {
		if (g_theJobSystem) {
			Job* claimedJob = g_theJobSystem->ClaimJob(m_channels);
			if (claimedJob) {
				claimedJob->Execute();
				g_theJobSystem->PostJob(claimedJob->m_id);
			}
		}
	}
}
