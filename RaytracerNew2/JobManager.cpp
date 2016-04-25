#include "JobManager.h"

#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Logger.h"

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
JobManager::JobManager()
:myThreadNumber(1)
,myNumberRunning(0)
{
#ifdef VS_2010
	myMutex = new sf::Mutex;
	myRemainingTasksMutex = new sf::Mutex;
#else
	//myMutex = new std::mutex;
	//myRemainingTasksMutex = new std::mutex;
#endif
	
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
JobManager::~JobManager(void)
{
	//delete myMutex;
	//delete myRemainingTasksMutex;

	destroyThreads();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::setThreadNumber(int number)
{
#ifdef VS_2010
	sf::Lock lock(*myRemainingTasksMutex);
#else
	std::lock_guard<std::mutex> lock(myRemainingTasksMutex);
#endif
	if(myNumberRunning > 0)
	{
		ILogger::log() << "Cannot change the number of threads while some tasks are running.\n";
		return;
	}

	destroyThreads();

	myThreadNumber = number;

}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::destroyThreads()
{
	for(unsigned int i = 0; i < myThreads.size(); i++)
	{
#ifdef VS_2010
		myThreads[i]->Wait();
		delete myThreads[i];
#else
		if(myThreads[i]->joinable())
			myThreads[i]->join();
		//delete myThreads[i];
#endif
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
bool JobManager::isFinished()
{
#ifdef VS_2010
	sf::Lock lock(*myRemainingTasksMutex);
#else
	std::lock_guard<std::mutex> lock(myRemainingTasksMutex);
#endif
	return myNumberRunning <= 0;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
int JobManager::remainingTasks()
{
#ifdef VS_2010
	sf::Lock lock(*myRemainingTasksMutex);
#else
	std::lock_guard<std::mutex> lock(myRemainingTasksMutex);
#endif
	return myNumberRunning;
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::addJobs(const std::vector<std::shared_ptr<Job> >& jobs)
{
	Timer clock;

	//Enqueue jobs
	for(unsigned int i = 0; i < jobs.size(); i++)
	{
		myJobs.push_back(jobs[i]);
	}

	//Single threaded processing
	if(myThreadNumber == 1)
	{
		myNumberRunning = myThreadNumber;
		for(unsigned int i = 0; i < jobs.size(); i++)
		{
			myJobs[i]->run();
		}
		myNumberRunning = 0;
	}
	else
	{
		if(myThreads.size() == 0) 
		{
			initJobs();
		}

		join();
	}

	ILogger::log() << "Render time : " << clock.elapsedTime() << " s.\n";
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::initJobs()
{
	myNumberRunning = myThreadNumber;
	myThreads.resize(myThreadNumber);

	for(int i = 0; i < myThreadNumber; i++) 
	{
#ifdef VS_2010
		myThreads[i] = new sf::Thread(&JobManager::jobRun, this);
		myThreads[i]->Launch();
#else
		myThreads[i].reset(new std::thread(&JobManager::jobRun, this));
#endif
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::join()
{
	for (int i = 0; i < myThreadNumber; i++)
	{
		myThreads[i]->join();
	}
}

//void JobManager::stopJobs()
//{
//	for(unsigned int i = 0; i < myJobs.size(); i++)
//	{
//		myJobs[i]->stop();
//	}
//
//	for(unsigned int i = 0; i < myThreadNumber; i++) 
//	{
//		myThreads[i]->Wait();
//	}
//}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
void JobManager::jobRun()
{
	while(true)
	{
		std::shared_ptr<Job> job = NULL;
		{	
#ifdef VS_2010
			sf::Lock lock(*myMutex);
#else
			std::lock_guard<std::mutex> lock(myMutex);
#endif
			if(myJobs.size() == 0)
				break;
	
			job = myJobs.back();
			myJobs.pop_back();
		}

		job->run();
	}

#ifdef VS_2010
	sf::Lock lock(*myRemainingTasksMutex);
#else
	std::lock_guard<std::mutex> lock(myRemainingTasksMutex);
#endif
	myNumberRunning--;
}