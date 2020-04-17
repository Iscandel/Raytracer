#ifndef H__THREADPOOL_160420202315_H
#define H__THREADPOOL_160420202315_H
#include "tools/Common.h"
#include "thread/TaskSet.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
	ThreadPool(int nbThreads = -1);
	~ThreadPool();

	void stop(bool joinMode = true);

	void reset(int nbThreads);

	void addTask(TaskSet::Function function, int nbSubTasks, bool waitForComplete);

	void cancel();
	
	int getThreadNumber() const { return myThreads.size(); }

protected:
	void run();
	std::pair<TaskSet::ptr, int> nextTask();

protected:
	std::mutex myRunningMutex;
	std::condition_variable myCondition;
	std::vector<std::unique_ptr<std::thread>> myThreads;
	std::queue<TaskSet::ptr> myTasks;

	std::atomic<bool> myIsRunning;
};

namespace thread
{
	extern ThreadPool* pool;

	inline void startPool()
	{
		pool = new ThreadPool(getCoreNumber());
	}

	inline void cleanup() {
		delete pool;
	}
}

#endif