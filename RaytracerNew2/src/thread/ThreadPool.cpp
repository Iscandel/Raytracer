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

void ThreadPool::run()
{
	while(myIsRunning)
	{
		std::shared_ptr<TaskSet> task;
		int subTaskId;
		{
			std::unique_lock<std::mutex> lock(myRunningMutex);
			myCondition.wait(lock, [&]() {return !myIsRunning || !myTasks.empty();});
			auto res  = nextTask(); //structured binding declaration to add with c++17: auto [task, subTaskId]
			task = res.first;
			subTaskId = res.second;
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
#include <chrono>
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

	using namespace std::chrono_literals;
	if (waitForComplete) {
		while (!task->finished()) {}
	}
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