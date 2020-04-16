#ifndef H__TASKSET_160420202314_H
#define H__TASKSET_160420202314_H

#include "tools/Common.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

class TaskSet
{
public:
	typedef std::shared_ptr<TaskSet> ptr;
	typedef std::function<void(int)> Function;

public:
	TaskSet(Function function, int nbTasks)
		:myFunction(std::move(function))
		, myNumberTasks(nbTasks)
		, myTasksRemaining(nbTasks)
		, myTasksThatFinished(0)
		, myIsFinished(false)
	{
	}

	int startTask() {
		return --myTasksRemaining;
	}

	int remainingSubTasks() const {
		return myTasksRemaining;
	}

	bool isLastSubTask() const {
		return myTasksRemaining == 0;
	}

	bool finished() {
		return myIsFinished;
	}

	void run(int subTaskId) {
		try {
			myFunction(subTaskId);
		}
		catch (std::exception& ex) {
			ILogger::log() << ex.what() << "\n";
		}

		int tmp = ++myTasksThatFinished;

		if (tmp == myNumberTasks) {
			//myCallback();
			//std::cout << "finito" << std::endl;
			myIsFinished = true;
			std::unique_lock<std::mutex> lock(myMutex);
			myCondition.notify_all();//.notify_one();
		}
	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(myMutex);
		myCondition.wait(lock);
	}

protected:
	int myNumberTasks;
	std::atomic<int> myTasksRemaining; //Tasks which have started
	std::atomic<int> myTasksThatFinished;
	std::atomic<bool> myIsFinished;
	Function myFunction;
	std::mutex myMutex;
	std::condition_variable myCondition;
};

#endif