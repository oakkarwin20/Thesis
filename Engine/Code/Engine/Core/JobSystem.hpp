#pragma once

#include <queue>
#include <vector>
#include <mutex>

//----------------------------------------------------------------------------------------------------------------------
class JobWorker;

//----------------------------------------------------------------------------------------------------------------------
enum JobStatus
{
	JOB_STATUS_NEW,				// Constructed but not queued for work yet
	JOB_STATUS_QUEUED,			// Queued, waiting to be claimed by a worker thread
	JOB_STATUS_WORKING,			// Claimed by a worker thread who is currently executing it
	JOB_STATUS_COMPLETED,		// Completed, placed into the completed list for the main thread
	JOB_STATUS_RETRIEVED,		// Retrieved by the main thread, retired from the Job system
};

//----------------------------------------------------------------------------------------------------------------------
class Job
{
public:
	Job() {};
	virtual ~Job() {};
	virtual void Execute() = 0;

	std::atomic<JobStatus>	m_jobStatus = JOB_STATUS_NEW;
};

//----------------------------------------------------------------------------------------------------------------------
struct JobSystemConfig
{
	int m_preferredNumberOfWorkers = -1;		// -1 means "one fewer than number of CPU cores"
};

//----------------------------------------------------------------------------------------------------------------------
class JobSystem
{
public:
	JobSystem( JobSystemConfig const& config )
		: m_config( config )
	{}

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void PostNewJob( Job* newJob );						// called by Main thread to add Job to ToDo list

//private:
	bool IsQuitting() const;
	void CreateNewWorkers( int numWorkerThreads );
	void DestroyAllWorkers();
	Job* ClaimJobForWorkerThread();						// Called by the main thread to put INTO the system AND (give up ownership)		//Claim Job
	void AddJobToCompletedList( Job* jobToDo );
	Job* RetrieveCompletedJob();						// Called by the main thread to get a Job back OUT of the system AND (retake ownership)

	void ClearAllJobListsAndJoinAllWorkers();
	void ClearUnclaimedJobslist();
	void ClearClaimedJobslist();
	void ClearCompletedJobslist();
	void WaitForWorkerThreadToJoin();

private:
	JobSystemConfig			m_config;
	std::atomic<bool>		m_isQuitting = false;

	std::vector<JobWorker*> m_workerList;

	std::queue<Job*>		m_unclaimedJobsList;		// List of jobs posted but not claimed by any worker (Work that needs to be done)
	std::mutex				m_unclaimedJobsListMutex;

	std::vector<Job*>		m_claimedJobsList;			// list of jobs currently claimed by workers (Work in progress)
	std::mutex				m_claimedJobsListMutex;

	std::queue<Job*>		m_completedJobsList;		// List of jobs finished, ready to be retrieved (Work completed)
	std::mutex				m_completedJobsListMutex;
};

//----------------------------------------------------------------------------------------------------------------------
class JobWorker
{
public:
	JobWorker( JobSystem* jobSystem, int threadID );
	~JobWorker();

	static void ThreadMain( JobWorker* jobWorker, int threadID );

	JobSystem*		m_jobSystem		= nullptr;
	int				m_threadID		= -1;				// Worker ID
	std::thread*	m_thread		= nullptr;			// This pointer will point to a thread created by the main thread
};
