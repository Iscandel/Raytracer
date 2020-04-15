#include "ThreadPool.h"



ThreadPool::ThreadPool(int nbThreads)
{
	reset(nbThreads);
}


ThreadPool::~ThreadPool()
{
	stop(false);
}

void ThreadPool::reset(int nbThreads)
{
	stop();

	if (nbThreads == -1) {
		nbThreads = getCoreNumber();
	}

	for (int i = 0; i < nbThreads; i++)
		myThreads.emplace_back(new std::thread(&ThreadPool::run, this));
}

void ThreadPool::run()
{
	while(!myIsRunning)
	{
		TaskSet& task = nextTask();

		{
			std::unique_lock<std::mutex> lock(myRunningMutex);
			myCondition.wait(lock, [&]() {!myIsRunning || !myTasks.empty();});
		}

		task.run();
	}
}

TaskPool& ThreadPool::nextTask()
{

}

void ThreadPool::addTask(TaskSet::Function function, int nbSubTasks, bool waitForComplete)
{
	TaskSet task(std::move(function), nbSubTasks);
	std::lock_guard<std::mutex> lock(myRunningMutex);
	myTasks.push(task);

	if (waitForComplete)
		myTasks.wait();
}

void ThreadPool::stop(bool joinMode)
{
	while (!myThreads.empty()) {
		joinMode ? myThreads.back()->join() : myThreads.back()->detach();
		myThreads.pop_back();
	}
}