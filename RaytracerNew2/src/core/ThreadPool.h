#pragma once
#include "tools/Common.h"
//#include "core/TaskGroup.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Barrier
{

};

class TaskSet
{
public:
	typedef std::function<void()> Function;
	
public:
	TaskSet(Function function, int nbTasks)
	{
		myFunction = std::move(function);
		myNumberTasks = nbTasks;
		myTasksRemaining = nbTasks;
	}

	void run() { myFunction(); }

	void wait()
	{

	}

protected:
	int myNumberTasks;
	std::atomic<int> myTasksRemaining;
	Function myFunction;
};

class ThreadPool
{
public:
	ThreadPool(int nbThreads = -1);
	~ThreadPool();

	void stop(bool joinMode = true);

	void reset(int nbThreads);

protected:
	void run();

	TaskSet& nextTask();

	void addTask(TaskSet::Function function, int nbSubTasks, bool waitForComplete);

protected:
	std::mutex myRunningMutex;
	std::condition_variable myCondition;
	std::vector<std::unique_ptr<std::thread>> myThreads;
	std::queue<TaskSet> myTasks;

	std::atomic<bool> myIsRunning;
};

