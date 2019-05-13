#pragma once
#include "Engine/Core/type.hpp"
#include "Engine/Core/Thread.hpp"
#include <vector>
#include <string>

class JobWorkerThread {
	friend class JobSystem;
public:
	JobWorkerThread(const std::string& name, u32 channels);
	~JobWorkerThread();

	bool IsRunning() const { return m_isRunning; }
	void JobWorkerThreadEntryFunction();

private:
	std::string			m_name = "UNNAMED WORKER THREAD";
	u32					m_channels = 0xFFFFFFFF;
	threadHandle		m_threadHandle;
	bool				m_isRunning = true;
};

// enum eJobStatus {
// 	JOB_STATUS_QUEUED = 0,
// 	JOB_STATUS_RUNNING,
// 	JOB_STATUS_COMPLETE,
// 	NUM_JOB_STATUS
// };

class Job {
public:
	Job(u32 channels = 0xFFFFFFFF, int jobType = -1) 
		: m_channels(channels), m_type(jobType) {
		m_id = s_globalID++;
		if (s_globalID < 0) {
			s_globalID = 0;
		}
	}
	virtual ~Job() = default;
	virtual void Execute() = 0;
	virtual void JobCompleteCallback() = 0;

public:
	int			m_id = -1;
	int			m_type = -1;
	u32			m_channels = 0xFFFFFFFF;
/*	eJobStatus	m_status;*/

	static int  s_globalID;
};

class JobSystem {
	friend class JobWorkerThread;
public:
	JobSystem();
	~JobSystem();

	void FinishAllCompletedJobs();
	void QueueJob(Uptr<Job> job);
	void CreateWorkerThread(const std::string& name, u32 channels = 0xFFFFFFFF);
	void DestroyWorkerThread(const std::string& name);

private:
	Job* ClaimJob(u32 channels);
	void PostJob(int jobID);

private:
	std::vector<JobWorkerThread>	m_workerThreads;

	std::mutex						m_queuedJobsMutex;
	std::vector<Uptr<Job>>			m_queuedJobs;

	std::mutex						m_runningJobsMutex;
	std::vector<Uptr<Job>>			m_runningJobs;

	std::mutex						m_completedJobsMutex;
	std::vector<Uptr<Job>>			m_completedJobs;
};
extern Uptr<JobSystem> g_theJobSystem;