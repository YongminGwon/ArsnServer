#include "pch.h"
#include "ThreadManager.h"

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void()> callback)
{
	scoped_lock guard(lock_);
	threads_.emplace_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (thread& t : threads_)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	threads_.clear();
}

void ThreadManager::InitTLS()
{
	static atomic<uint32> ThreadId = 1;
	LThreadId = ThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{
}
