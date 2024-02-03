#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

//----------------------------------------------------------------------------------------------------------------------
JobSystem* g_theJobSystem = nullptr;

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::Startup()
{
	int numWorkers = m_config.m_preferredNumberOfWorkers;
	if ( numWorkers < 0 )
	{
		int numCpuCores = std::thread::hardware_concurrency();
		numWorkers = numCpuCores - 1;
	}
	CreateNewWorkers( numWorkers );
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::BeginFrame()
{
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::EndFrame()
{
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
	m_isQuitting = true;
	DestroyAllWorkers();
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::PostNewJob( Job* newJob )
{
	m_unclaimedJobsListMutex.lock();
	m_unclaimedJobsList.push( newJob );
	newJob->m_jobStatus = JOB_STATUS_QUEUED;
	m_unclaimedJobsListMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
bool JobSystem::IsQuitting() const
{
	return m_isQuitting;
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::CreateNewWorkers( int numWorkerThreads )
{
	for ( int i = 0; i < numWorkerThreads; i++ )
	{
		JobWorker* jobWorker = new JobWorker( this, i );
		m_workerList.push_back( jobWorker );
	}
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::DestroyAllWorkers()
{
	for ( int i = 0; i < m_workerList.size(); i++ )
	{
		delete m_workerList[i];
	}
	m_workerList.clear();
}

//----------------------------------------------------------------------------------------------------------------------
Job* JobSystem::ClaimJobForWorkerThread()
{
	Job* jobToDo = nullptr;
	m_unclaimedJobsListMutex.lock();
	if ( !m_unclaimedJobsList.empty() )
	{
		jobToDo = m_unclaimedJobsList.front();				// Get Front
		m_unclaimedJobsList.pop();							// Pop "front"
		jobToDo->m_jobStatus = JOB_STATUS_WORKING;		// Change this job's status to "in-progress" (being worked on by the thread)

//		m_claimedJobsListMutex.lock();
//		m_claimedJobsList.push_back( jobToDo );
//		m_claimedJobsListMutex.unlock();
	}
	m_unclaimedJobsListMutex.unlock();

	return jobToDo;
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::AddJobToCompletedList( Job* jobToDo )
{
	m_completedJobsListMutex.lock();
	m_completedJobsList.push( jobToDo );
	jobToDo->m_jobStatus = JOB_STATUS_COMPLETED;
	m_completedJobsListMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
Job* JobSystem::RetrieveCompletedJob()
{
	Job* completedJob = nullptr;
	m_completedJobsListMutex.lock();
	if ( !m_completedJobsList.empty() )
	{
		completedJob = m_completedJobsList.front();
		m_completedJobsList.pop();
		completedJob->m_jobStatus = JOB_STATUS_RETRIEVED;			// No longer owned by the job system, ownership has been given BACK to the main thread
	}
	m_completedJobsListMutex.unlock();
	return completedJob;
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearAllJobListsAndJoinAllWorkers()
{
	ClearUnclaimedJobslist();
	ClearCompletedJobslist();
	WaitForWorkerThreadToJoin();
	ClearClaimedJobslist();
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearUnclaimedJobslist()
{
	m_unclaimedJobsListMutex.lock();
	for ( int i = 0; i < m_unclaimedJobsList.size(); i++ )
	{
		m_unclaimedJobsList.pop();
	}
	m_unclaimedJobsListMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearClaimedJobslist()
{
	m_claimedJobsListMutex.lock();
	m_claimedJobsList.clear();
	m_claimedJobsListMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::ClearCompletedJobslist()
{
	m_completedJobsListMutex.lock();
	while ( m_completedJobsList.size() != 0 )
	{
		m_completedJobsList.pop();
	}
	m_completedJobsListMutex.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void JobSystem::WaitForWorkerThreadToJoin()
{
	if ( m_claimedJobsList.size() > 0 )
	{
		for ( int i = 0; i < m_workerList.size(); i++ )
		{
			m_workerList[i]->m_thread->join();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
JobWorker::JobWorker( JobSystem* jobSystem, int threadID )
	: m_jobSystem( jobSystem )
	, m_threadID( threadID )
{
	m_thread = new std::thread( JobWorker::ThreadMain, this, m_threadID );
}

//----------------------------------------------------------------------------------------------------------------------
JobWorker::~JobWorker()
{
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void JobWorker::ThreadMain( JobWorker* jobWorker, int m_threadID )
{
	UNUSED( m_threadID );

	while ( !jobWorker->m_jobSystem->IsQuitting() )
	{
		Job* jobToDo = jobWorker->m_jobSystem->ClaimJobForWorkerThread();
		if ( jobToDo != nullptr )
		{
			jobToDo->Execute();													// Actually chunk::Generate();
			jobWorker->m_jobSystem->AddJobToCompletedList( jobToDo );
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds(1) );		// Don't hog the CPU just checking for work
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
// void ThreadMain( int threadID, JobSystem* jobSystem )
// {
// 	while ( !jobSystem->isQuitting )
// 	{
// 		Job* jobToDo = jobSystem->GetJobToWorkOn();
// 		if ( jobToDo != nullptr )
// 		{
// 			jobToDo->Execute();
// 		}
// 		else
// 		{
// 			//			std::this_thread::sleep_for( std::chrono:: );
// 		}
// 
// 	}
// }
