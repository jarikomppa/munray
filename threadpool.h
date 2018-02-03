#pragma once
#ifndef THREADPOOL_H
#define THREADPOOL_H
#define MAX_THREADPOOL_TASKS 1024

namespace Thread
{
	typedef void(*threadFunction)(void *aParam);

	struct ThreadHandleData;
	typedef ThreadHandleData* ThreadHandle;

	void * createMutex();
	void destroyMutex(void *aHandle);
	void lockMutex(void *aHandle);
	void unlockMutex(void *aHandle);

	ThreadHandle createThread(threadFunction aThreadFunction, void *aParameter);

	void sleep(int aMSec);
	void wait(ThreadHandle aThreadHandle);
	void release(ThreadHandle aThreadHandle);

	void * createMutex();
	void destroyMutex(void *aHandle);
	void lockMutex(void *aHandle);
	void unlockMutex(void *aHandle);

	struct thread_data
	{
		threadFunction mFunc;
		void *mParam;
	};

	ThreadHandle createThread(threadFunction aThreadFunction, void *aParameter);
	void sleep(int aMSec);
	void wait(ThreadHandle aThreadHandle);
	void release(ThreadHandle aThreadHandle);
	void poolWorker(void *aParam);

	class PoolTask
	{
	public:
		virtual void work() = 0;
	};


	class Pool
	{
	public:
		volatile int mRunning; // running flag, used to flag threads to stop (used by dtor)
		int mThreadCount; // number of threads
		ThreadHandle *mThread; // array of thread handles
		void *mWorkMutex; // mutex to protect task array/maxtask
		PoolTask *mTaskArray[MAX_THREADPOOL_TASKS]; // pointers to tasks
		int mMaxTask; // how many tasks are pending
		int mTasksRunning; // how many tasks are running
		int mRobin; // cyclic counter, used to pick jobs for threads

		// Initialize and run thread pool. For thread count 0, work is done at addWork call.
		void init(int aThreadCount);
		// Ctor, sets known state
		Pool();
		// Dtor. Waits for the threads to finish. Work may be unfinished.
		~Pool();
		// Add work to work list. Object is not automatically deleted when work is done.
		void addWork(PoolTask *t);
		// Call to wait until all tasks are done.
		void waitUntilDone();
		// Called from worker thread to get a new task. Returns null if no work available.
		PoolTask *getWork();
		// Called from worker thread to signal that one bit of work is done.
		void doneWork();
	};
}
#endif