#include "ThreadPool.h"



ThreadPool::ThreadPool(int nbThreads)
:myIsRunning(false)
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

	myIsRunning = true;

	for (int i = 0; i < nbThreads; i++)
		myThreads.emplace_back(new std::thread(&ThreadPool::run, this));
}

void ThreadPool::cancel() {
	myIsRunning = false;
	std::unique_ptr<std::mutex> lock;
	myCondition.notify_all();
}

void ThreadPool::run()
{
	while(myIsRunning)
	{
		std::shared_ptr<TaskSet> task;
		int subTaskId;
		{
			std::unique_lock<std::mutex> lock(myRunningMutex);
			myCondition.wait(lock, [&]() {return !myIsRunning || !myTasks.empty();});
			std::tie(task, subTaskId) = nextTask(); //structured binding declaration to add with c++17: auto [task, subTaskId]
			//task = std::move(res.first);
			//subTaskId = res.second;
		}

		if(task)
			task->run(subTaskId);
	}
}

std::pair<TaskSet::ptr, int> ThreadPool::nextTask()
{
	if (!myIsRunning)
		return{ TaskSet::ptr(), 0 };

	//Shouldn't be empty here
	TaskSet::ptr task = myTasks.front();

	int subTaskId = task->startTask();
	if (task->isLastSubTask()) {
		myTasks.pop();
	}
	
	return{ task, subTaskId };
}

void ThreadPool::addTask(TaskSet::Function function, int nbSubTasks, bool waitForComplete)
{
	TaskSet::ptr task(new TaskSet(std::move(function), nbSubTasks));
	{
		std::lock_guard<std::mutex> lock(myRunningMutex);
		myTasks.push(task);

		if (nbSubTasks == 1)
			myCondition.notify_one();
		else
			myCondition.notify_all();
	}

	if (waitForComplete) {
		while (!task->finished()) {}
	}

	//not working...
	//if (waitForComplete)
	//	//std::this_thread::sleep_for(20s);
	//	task->wait();
	//std::cout << "Et ca vaut " << task->remainingSubTasks() << std::endl;
}

void ThreadPool::stop(bool joinMode)
{
	if (!myIsRunning)
		return;

	myIsRunning = false;
	{
		std::lock_guard<std::mutex> lock(myRunningMutex);
		myCondition.notify_all();
	}
	while (!myThreads.empty()) {
		joinMode ? myThreads.back()->join() : myThreads.back()->detach();
		myThreads.pop_back();
	}
}

namespace thread
{
	ThreadPool* pool = nullptr;
}