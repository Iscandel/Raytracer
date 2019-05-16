#pragma once

#include "Job.h"

#include <vector>
#include <queue>

#ifdef VS_2010
#include <SFML/System.hpp>
#else
#include <thread>
#include <mutex>
#endif

class Camera;
class Sampler;
class Scene;

///////////////////////////////////////////////////////////////////////////////
/// \brief This class allows multithreaded computation of image chunks
///////////////////////////////////////////////////////////////////////////////
class JobManager
{
public:
	JobManager();
	~JobManager(void);

	///////////////////////////////////////////////////////////////////////////////
	/// Defines the number of threads to be used.
	///////////////////////////////////////////////////////////////////////////////
	void setThreadNumber(int number);

	///////////////////////////////////////////////////////////////////////////////
	/// Returns the number of threads to be used.
	/// \return Number of threads.
	///////////////////////////////////////////////////////////////////////////////
	int getThreadNumber() const {return myThreadNumber;}

	///////////////////////////////////////////////////////////////////////////////
	/// Returns the number of active tasks (threads).
	///////////////////////////////////////////////////////////////////////////////
	int remainingTasks();

	///////////////////////////////////////////////////////////////////////////////
	/// Pauses the program execution until the end of all the jobs.
	///////////////////////////////////////////////////////////////////////////////
	void join();

	bool isFinished();

	void addJobs(const std::vector<std::shared_ptr<Job> >& jobs);

	//void stopJobs();

	void initJobs();
	
	void jobRun();

	void destroyThreads();

	void showProgress();

	void setShowProgress(bool show) { myIsShowProgress = show; }


	void run(int myOffsetX, int myOffsetY, Camera::ptr myCamera, int mySizeX, int mySizeY, std::shared_ptr<Sampler> mySampler, Scene* myScene);

protected:
	int myThreadNumber;
#ifdef VS_2010
	std::vector<sf::Thread*> myThreads;
	sf::Mutex* myMutex ;
	sf::Mutex* myRemainingTasksMutex ;
	sf::Mutex* myProgressMutex;
#else
	std::vector<std::shared_ptr<std::thread> > myThreads;
	std::mutex myMutex;
	std::mutex myRemainingTasksMutex;
	std::mutex myProgressMutex;
#endif
	int myNumberRunning;
	std::vector<std::shared_ptr<Job> > myJobs;

	bool myIsShowProgress;

	int myJobsDone;
	int myTotalJobs;
	int myCoeff;
	real myShowValue;
};


