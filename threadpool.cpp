#include <Windows.h>

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

	struct ThreadHandleData
	{
		HANDLE thread;
	};

	void * createMutex()
	{
		CRITICAL_SECTION * cs = new CRITICAL_SECTION;
		InitializeCriticalSectionAndSpinCount(cs, 100);
		return (void*)cs;
	}

	void destroyMutex(void *aHandle)
	{
		CRITICAL_SECTION *cs = (CRITICAL_SECTION*)aHandle;
		DeleteCriticalSection(cs);
		delete cs;
	}

	void lockMutex(void *aHandle)
	{
		CRITICAL_SECTION *cs = (CRITICAL_SECTION*)aHandle;
		if (cs)
		{
			EnterCriticalSection(cs);
		}
	}

	void unlockMutex(void *aHandle)
	{
		CRITICAL_SECTION *cs = (CRITICAL_SECTION*)aHandle;
		if (cs)
		{
			LeaveCriticalSection(cs);
		}
	}

	struct thread_data
	{
		threadFunction mFunc;
		void *mParam;
	};

	static DWORD WINAPI threadfunc(LPVOID d)
	{
		thread_data *p = (thread_data *)d;
		p->mFunc(p->mParam);
		delete p;
		return 0;
	}

	ThreadHandle createThread(threadFunction aThreadFunction, void *aParameter)
	{
		thread_data *d = new thread_data;
		d->mFunc = aThreadFunction;
		d->mParam = aParameter;
		HANDLE h = CreateThread(NULL, 0, threadfunc, d, 0, NULL);
		if (0 == h)
		{
			return 0;
		}
		ThreadHandleData *threadHandle = new ThreadHandleData;
		threadHandle->thread = h;
		return threadHandle;
	}

	void sleep(int aMSec)
	{
		Sleep(aMSec);
	}

	void wait(ThreadHandle aThreadHandle)
	{
		WaitForSingleObject(aThreadHandle->thread, INFINITE);
	}

	void release(ThreadHandle aThreadHandle)
	{
		CloseHandle(aThreadHandle->thread);
		delete aThreadHandle;
	}

	void poolWorker(void *aParam);

	class PoolTask
	{
	public:
		virtual void work() = 0;
	};

#define MAX_THREADPOOL_TASKS 1024

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

	void Pool::init(int aThreadCount)
	{
		if (aThreadCount > 0)
		{
			mMaxTask = 0;
			mWorkMutex = createMutex();
			mRunning = 1;
			mTasksRunning = 0;
			mThreadCount = aThreadCount;
			mThread = new ThreadHandle[aThreadCount];
			int i;
			for (i = 0; i < mThreadCount; i++)
			{
				mThread[i] = createThread(poolWorker, this);
			}
		}
	}

	Pool::Pool()
	{
		mRunning = 0;
		mThreadCount = 0;
		mThread = 0;
		mWorkMutex = 0;
		mRobin = 0;
		mMaxTask = 0;
	}

	Pool::~Pool()
	{
		mRunning = 0;
		int i;
		for (i = 0; i < mThreadCount; i++)
		{
			wait(mThread[i]);
			release(mThread[i]);
		}
		delete[] mThread;
		if (mWorkMutex)
			destroyMutex(mWorkMutex);
	}

	void Pool::addWork(PoolTask *aTask)
	{
		if (mThreadCount == 0)
		{
			aTask->work();
		}
		else
		{
			if (mWorkMutex) lockMutex(mWorkMutex);
			if (mMaxTask == MAX_THREADPOOL_TASKS)
			{
				// If we're at max tasks, do the task on calling thread 
				// (we're in trouble anyway, might as well slow down adding more work)
				if (mWorkMutex) unlockMutex(mWorkMutex);
				aTask->work();
			}
			else
			{
				mTaskArray[mMaxTask] = aTask;
				mMaxTask++;
				if (mWorkMutex) unlockMutex(mWorkMutex);
			}
		}
	}

	PoolTask * Pool::getWork()
	{
		PoolTask *t = 0;
		if (mWorkMutex) lockMutex(mWorkMutex);
		if (mMaxTask > 0)
		{
			int r = mRobin % mMaxTask;
			mRobin++;
			t = mTaskArray[r];
			mTaskArray[r] = mTaskArray[mMaxTask - 1];
			mMaxTask--;
			mTasksRunning++;
		}
		if (mWorkMutex) unlockMutex(mWorkMutex);
		return t;
	}

	void Pool::doneWork()
	{
		if (mWorkMutex) lockMutex(mWorkMutex);
		mTasksRunning--;
		if (mWorkMutex) unlockMutex(mWorkMutex);
	}

	void Pool::waitUntilDone()
	{
		while (mMaxTask + mTasksRunning) { Thread::sleep(1); }
	}

	void poolWorker(void *aParam)
	{
		Pool *myPool = (Pool*)aParam;
		while (myPool->mRunning)
		{
			PoolTask *t = myPool->getWork();
			if (!t)
			{
				sleep(1);
			}
			else
			{
				t->work();
				myPool->doneWork();
			}
		}
	}
}