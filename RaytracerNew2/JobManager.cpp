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
,myIsShowProgress(false)
,myJobsDone(0)
,myCoeff(0)
,myShowValue(0.1)
{
#ifdef VS_2010
	myMutex = new sf::Mutex;
	myRemainingTasksMutex = new sf::Mutex;
	myProgressMutex = new sf::Mutex;
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

	myTotalJobs = myJobs.size();

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
	//std::thread render_thread([&] {
	//	tbb::blocked_range<int> range(0, this->myThreadNumber);
	//	std::shared_ptr<Job> job = myJobs.back();

	//	auto map = [&](const tbb::blocked_range<int> &range) {

	//		for (int i = range.begin(); i < range.end(); ++i) {
	//			run(job->myOffsetX, job->myOffsetY, job->myCamera, job->mySizeX, job->mySizeY, job->mySampler, job->myScene);
	//		}
	//	};

	//	tbb::parallel_for(range, map);
	//});

	//render_thread.join();
	//return;

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
	int nbThreads = myThreads.size();
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

			if (myIsShowProgress)
			{
				showProgress();
			}
		}

		//run(job->myOffsetX, job->myOffsetY, job->myCamera, job->mySizeX, job->mySizeY, job->mySampler, job->myScene);
		job->run();
	}

#ifdef VS_2010
	sf::Lock lock(*myRemainingTasksMutex);
#else
	std::lock_guard<std::mutex> lock(myRemainingTasksMutex);
#endif
	myNumberRunning--;
}

void JobManager::showProgress()
{
#ifdef VS_2010
	sf::Lock lock(*myProgressMutex);
#else
	std::lock_guard<std::mutex> lock(myProgressMutex);
#endif
	myJobsDone++;

	if ((double)myJobsDone / myTotalJobs > myShowValue * myCoeff)
	{
		ILogger::log() << myShowValue * myCoeff * 100 << "%\n";
		myCoeff++;
	}
}

//void JobManager::run(int myOffsetX, int myOffsetY, Camera::ptr myCamera, int mySizeX, int mySizeY, std::shared_ptr<Sampler> mySampler, Scene* myScene)
//{
//	int startX = std::max(0, myOffsetX);
//	int endX = std::min(myCamera->getSizeX(), myOffsetX + mySizeX);
//	int startY = std::max(0, myOffsetY);
//	int endY = std::min(myCamera->getSizeY(), myOffsetY + mySizeY);
//
//	std::unique_ptr<Screen> subScreen =
//		myCamera->getScreen().createSubScreen(endX - startX, endY - startY, myOffsetX, myOffsetY);
//
//	for (int y = myOffsetY; y < endY; y++)
//	{
//		//std::cout << y << std::endl;
//		for (int x = myOffsetX; x < endX; x++)
//		{
//			//std::cout << "x " << x << std::endl;
//
//			for (int i = 0; i < mySampler->getSampleNumber(); i++)
//			{
//				Point2d sample = mySampler->getNextSample2D();
//				//472 113
//				//477 172
//				double xx = (double)x + sample.x();
//				double yy = (double)y + sample.y();
//				//double xx = 768;(double)x + sample.x();
//				//double yy = 550;(double)y + sample.y();
//				//				
//				//double xx = 313;(double)x + sample.x();
//				//double yy = 416;(double)y + sample.y();
//
//				Ray ray = myCamera->getRay(xx, yy, mySampler->getNextSample2D());
//
//				Color col = myScene->getIntegrator()->li(*myScene, mySampler, ray);
//
//				subScreen->addSample(xx, yy, col);
//				//myCamera->getScreen().addSample(xx, yy, col);
//			}
//		}
//	}
//
//	//Merge the subblock with the full screen
//	myCamera->getScreen().merge(*subScreen);
//}



/*	omp_set_num_threads(myThreadNumber);
	int nThreads = 0;
#pragma omp parallel default(none) private(i) shared(myJobs, nThreads)
	{
#pragma omp master  
		nThreads = omp_get_num_threads();
#pragma omp for
		for (int i = 0; i < myJobs.size(); i++)
		{
			myJobs[i]->run();
		}
		}
*/